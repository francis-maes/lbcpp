/*-----------------------------------------.---------------------------------.
| Filename: BanditsSandBox.h               | Bandits Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

# include "../Problem/DiscreteBanditDecisionProblem.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>

namespace lbcpp
{

  
///////////////////////////////////////////////////
//////// Bandit policies base classes /////////////
///////////////////////////////////////////////////
  
extern ClassPtr banditStatisticsClass;

class BanditStatistics : public Object
{
public:
  BanditStatistics() : Object(banditStatisticsClass), statistics(new ScalarVariableStatistics(T("reward"))) {}

  void update(double reward)
    {statistics->push(reward);}

  size_t getPlayedCount() const
    {return (size_t)statistics->getCount();}

  double getRewardMean() const
    {return statistics->getMean();}

  double getSquaredRewardMean() const
    {return statistics->getSquaresMean();}

  double getRewardVariance() const
    {return statistics->getVariance();}

  double getRewardStandardDeviation() const
    {return statistics->getStandardDeviation();}

  double getMinReward() const
    {return statistics->getMinimum();}

  double getMaxReward() const
    {return statistics->getMaximum();}

private:
  friend class BanditStatisticsClass;

  ScalarVariableStatisticsPtr statistics;
};

typedef ReferenceCountedObjectPtr<BanditStatistics> BanditStatisticsPtr;

class DiscreteBanditPolicy : public Object
{
public:
  virtual void initialize(size_t numBandits)
  {
    timeStep = 0;
    banditStatistics.resize(numBandits);
    for (size_t i = 0; i < numBandits; ++i)
      banditStatistics[i] = new BanditStatistics();
  }

  size_t selectNextBandit()
    {return selectBandit(timeStep, banditStatistics);}

  void updatePolicy(size_t banditNumber, double reward)
  {
    jassert(banditNumber < banditStatistics.size());
    ++timeStep;
    banditStatistics[banditNumber]->update(reward);
  }

protected:
  friend class DiscreteBanditPolicyClass;

  size_t timeStep;
  std::vector<BanditStatisticsPtr> banditStatistics;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) = 0;
};

typedef ReferenceCountedObjectPtr<DiscreteBanditPolicy> DiscreteBanditPolicyPtr;

class IndexBasedDiscreteBanditPolicy : public DiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const = 0;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    if (timeStep < banditStatistics.size())
      return timeStep; // play each bandit once
    return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

  size_t selectMaximumIndexBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
    for (size_t i = 0; i < banditStatistics.size(); ++i)
    {
      double score = computeBanditScore(i, timeStep, banditStatistics);
      if (score > bestScore)
        bestBandit = i, bestScore = score;
    }
    return bestBandit;
  }
};

///////////////////////////////////////////////////
/////////// Baseline Bandit Policies //////////////
///////////////////////////////////////////////////
class UCB1DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }
};

class UCB1TunedDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    size_t nj = statistics->getPlayedCount();
    double lnn = log((double)timeStep);
    double varianceUB = statistics->getRewardVariance() + sqrt(2 * lnn / nj);
    return statistics->getRewardMean() + sqrt((lnn / nj) * juce::jmin(0.25, varianceUB));
  }
};

class UCB2DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  UCB2DiscreteBanditPolicy(double alpha = 1.0) : alpha(alpha) {}
 
  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    episodeCounts.clear();
    episodeCounts.resize(numBandits, 0);
    episodeRemainingSteps = 0;
    currentBandit = 0;
  }

protected:
  friend class UCB2DiscreteBanditPolicyClass;

  double alpha;
  std::vector<size_t> episodeCounts;
  size_t episodeRemainingSteps;
  size_t currentBandit;

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tauEpisodeCount = tau(episodeCounts[banditNumber]);
    double e = 1.0;//2.71828183;
    return statistics->getRewardMean() + sqrt((1.0 + alpha) * log(e * timeStep / tauEpisodeCount) / (2.0 * tauEpisodeCount));
  }

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    while (episodeRemainingSteps == 0)
    {
      currentBandit = selectMaximumIndexBandit(timeStep, banditStatistics);
      size_t& rj = episodeCounts[currentBandit];
      episodeRemainingSteps = tau(rj + 1) - tau(rj);
      ++rj;
    }

    --episodeRemainingSteps;
    return currentBandit;
  }

  size_t tau(size_t count) const
    {return (size_t)ceil(pow(1 + alpha, (double)count));}
};

class EpsilonGreedyDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  EpsilonGreedyDiscreteBanditPolicy(double c, double d)
    : c(c), d(d), random(new RandomGenerator()) {}
  EpsilonGreedyDiscreteBanditPolicy() : c(0.0), d(0.0) {}

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    random->setSeed(16645186);
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[banditNumber]->getRewardMean();}
 
  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    double epsilon = juce::jmin(1.0, c * numBandits / (d * d * (timeStep + 1)));
    if (random->sampleBool(epsilon))
      return random->sampleSize(numBandits);
    else
      return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

protected:
  friend class EpsilonGreedyDiscreteBanditPolicyClass;

  double c;
  double d;
  RandomGeneratorPtr random;
};

///////////////////////////////////////////////////
/////////// Optimized Bandit Policy ///////////////
///////////////////////////////////////////////////
class BanditStatisticsFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return banditStatisticsClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("banditStatisticFeatures"));
    res->addElement(context, T("log10(playedCount)"));
    res->addElement(context, T("mean(reward)"));
    res->addElement(context, T("mean(reward^2)"));
    res->addElement(context, T("stddev(reward)"));
    res->addElement(context, T("min(reward"));
    res->addElement(context, T("max(reward"));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const BanditStatisticsPtr& bandit = inputs[0].getObjectAndCast<BanditStatistics>();
    callback.sense(0, log10((double)bandit->getPlayedCount()));
    callback.sense(1, bandit->getRewardMean());
    callback.sense(2, bandit->getSquaredRewardMean());
    callback.sense(3, bandit->getRewardStandardDeviation());
    callback.sense(4, bandit->getMinReward());
    callback.sense(5, bandit->getMaxReward());
  }
};

// TimeStep, BanditStatistics -> Features
class DiscreteBanditPerception : public CompositeFunction
{
public:
  DiscreteBanditPerception(size_t maxTimeStep = 0)
    : maxTimeStep(maxTimeStep) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t timeStep = builder.addInput(positiveIntegerType, T("timeStep"));
    size_t banditStatistics = builder.addInput(banditStatisticsClass, T("banditStats"));

    //size_t timeFeatures = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0.0, log10((double)maxTimeStep), 3), timeStep);
    size_t timeFeature = builder.addFunction(convertToDoubleFunction(true), timeStep, T("time"));
    //timeFeatures = builder.addFunction(concatenateFeatureGenerator(), timeFeatures);

    size_t unitFeature = builder.addConstant(1.0, T("1"));
    size_t banditFeatures = builder.addFunction(new BanditStatisticsFeatureGenerator(), banditStatistics, T("bandit"));
    size_t stateFeatures = builder.addFunction(concatenateFeatureGenerator(false), timeFeature, unitFeature, banditFeatures, T("s"));

    builder.addFunction(cartesianProductFeatureGenerator(), stateFeatures, stateFeatures);
  }

protected:
  friend class DiscreteBanditPerceptionClass;

  size_t maxTimeStep;
};

class OptimizedDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  OptimizedDiscreteBanditPolicy(FunctionPtr perceptionFunction, DenseDoubleVectorPtr parameters)
    : perceptionFunction(perceptionFunction), parameters(parameters) {}
  OptimizedDiscreteBanditPolicy() {}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    DoubleVectorPtr perception = perceptionFunction->compute(defaultExecutionContext(), timeStep, banditStatistics[banditNumber]).getObjectAndCast<DoubleVector>();
    if (!perception)
      return 0.0;
    SparseDoubleVectorPtr features = perception->toSparseVector();
    return features->dotProduct(parameters, 0);
  }

protected:
  friend class OptimizedDiscreteBanditPolicyClass;

  FunctionPtr perceptionFunction;
  DenseDoubleVectorPtr parameters;
};

/*
** Evaluate functions
*/
class EvaluateDiscreteBanditPolicyWorkUnit : public WorkUnit
{
public:
  EvaluateDiscreteBanditPolicyWorkUnit(size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy, bool verbose = true)
    : numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), initialStatesDescription(initialStatesDescription), policy(policy), verbose(verbose) {}

  EvaluateDiscreteBanditPolicyWorkUnit()
    : numBandits(0), maxTimeStep(0), verbose(false) {}

  virtual String toShortString() const
    {return T("Evaluate ") + policy->getClass()->getShortName() + T("(") +  policy->toShortString() + T(") on ") + initialStatesDescription;}

  virtual Variable run(ExecutionContext& context)
  {
    // compute timeSteps
    std::vector<size_t> timeSteps;
    size_t timeStep = 0;
    size_t batchSize = 2;//numBandits;
    size_t batchIndex = 0;
    while (timeStep < maxTimeStep)
    {
      timeStep += batchSize;
      timeSteps.push_back(timeStep);
      ++batchIndex;
      batchSize *= 2;//numBandits;
    }
    
    // initial accumulators
    std::vector<double> actualRegretVector(timeSteps.size(), 0.0);
    std::vector<double> bestMachinePlayedVector(timeSteps.size(), 0.0);

    ScalarVariableStatisticsPtr actualRegretStatistics = new ScalarVariableStatistics(T("actualRegret")); 

    // main calculation loop
    for (size_t i = 0; i < initialStates.size(); ++i)
    {
      DiscreteBanditStatePtr state = initialStates[i]->cloneAndCast<DiscreteBanditState>();
      double bestReward, secondBestReward;
      size_t optimalBandit = state->getOptimalBandit(bestReward, secondBestReward);
      policy->initialize(numBandits);

      double sumOfRewards = 0.0;
      size_t numberOfTimesOptimalIsPlayed = 0;
      timeStep = 0;
      for (size_t j = 0; j < timeSteps.size(); ++j)
      {
        size_t numTimeSteps = timeSteps[j] - (j > 0 ? timeSteps[j - 1] : 0);
        for (size_t k = 0; k < numTimeSteps; ++k, ++timeStep)
           performBanditStep(state, policy, optimalBandit, sumOfRewards, numberOfTimesOptimalIsPlayed);
        jassert(timeStep == timeSteps[j]);
 
        actualRegretVector[j] += timeStep * bestReward - sumOfRewards;
        if (timeStep <= numBandits)
          bestMachinePlayedVector[j] += 1.0 / (double)numBandits;
        else
          bestMachinePlayedVector[j] += numberOfTimesOptimalIsPlayed / (double)timeStep;
      }
      actualRegretStatistics->push(timeStep * bestReward - sumOfRewards);

      if (verbose)
        context.progressCallback(new ProgressionState(i + 1, initialStates.size(), T("Problems")));
    }

    double invZ = 1.0 / initialStates.size();
    for (size_t i = 0; i < timeSteps.size(); ++i)
    {
      actualRegretVector[i] *= invZ;
      bestMachinePlayedVector[i] *= invZ;
      if (verbose)
      {
        size_t timeStep = timeSteps[i];
        context.enterScope(String((int)timeStep) + T(" Steps"));
        context.resultCallback(T("log10(n)"), log10((double)timeStep));
        context.resultCallback(T("bestMachinePlayed"), bestMachinePlayedVector[i] * 100.0);
        context.resultCallback(T("actualRegret"), actualRegretVector[i]);
        context.resultCallback(T("n"), timeStep);
        context.leaveScope(actualRegretVector[i]);
      }
    }

    return actualRegretStatistics;
  }
 
protected:
  friend class EvaluateDiscreteBanditPolicyWorkUnitClass;

  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  String initialStatesDescription;
  DiscreteBanditPolicyPtr policy;
  bool verbose;

  void performBanditStep(DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy, size_t optimalBandit, double& sumOfRewards, size_t& numberOfTimesOptimalIsPlayed)
  {
    size_t action = policy->selectNextBandit();
    double reward;
    state->performTransition(action, reward);
    policy->updatePolicy(action, reward);
    sumOfRewards += state->getExpectedReward(action);
    if (action == optimalBandit)
      ++numberOfTimesOptimalIsPlayed;
  }
};

class EvaluateOptimizedDiscreteBanditPolicyParameters : public SimpleUnaryFunction
{
public:
  EvaluateOptimizedDiscreteBanditPolicyParameters(FunctionPtr perception, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates)
    : SimpleUnaryFunction(denseDoubleVectorClass(), doubleType), perception(perception),
      numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), verbose(false)
  {
  }
  EvaluateOptimizedDiscreteBanditPolicyParameters() : SimpleUnaryFunction(variableType, variableType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DenseDoubleVectorPtr& parameters = input.getObjectAndCast<DenseDoubleVector>();
    DiscreteBanditPolicyPtr policy = new OptimizedDiscreteBanditPolicy(perception, parameters);
    WorkUnitPtr workUnit = new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Training Problems"), policy, verbose);
    return context.run(workUnit, false).toDouble();
  }

protected:
  friend class EvaluateOptimizedDiscreteBanditPolicyParametersClass;

  FunctionPtr perception;
  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  bool verbose;
};

/*
** SandBox
*/
class BanditsSandBox : public WorkUnit
{
public:
  BanditsSandBox() : numBandits(2), maxTimeStep(100000), numTrainingProblems(100), numTestingProblems(100) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    /*
    ** Make training and testing problems
    */
   // jassert(numBandits == 10);
    std::vector<double> probs(numBandits);
    const double rewardMargin = 0.1;

    std::vector<DiscreteBanditStatePtr> trainingStates(numTrainingProblems);
    for (size_t i = 0; i < trainingStates.size(); ++i)
    {
      for (size_t j = 0; j < probs.size(); ++j)
        probs[j] = random->sampleDouble();
      //std::random_shuffle(probs.begin(), probs.end());
      trainingStates[i] = new BernouilliDiscreteBanditState(probs, random->sampleUint32());
    }

    std::vector<DiscreteBanditStatePtr> testingStates(numTestingProblems);
    for (size_t i = 0; i < testingStates.size(); ++i)
    {
      for (size_t j = 0; j < probs.size(); ++j)
        probs[j] = random->sampleDouble();
      //std::random_shuffle(probs.begin(), probs.end());
      testingStates[i] = new BernouilliDiscreteBanditState(probs, random->sampleUint32());
    }

    /*
    ** Compute a bunch of baseline policies
    */
    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(new UCB2DiscreteBanditPolicy(0.001));
    policies.push_back(new UCB1TunedDiscreteBanditPolicy());
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.05, rewardMargin));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.10, rewardMargin));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.15, rewardMargin));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.20, rewardMargin));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.25, rewardMargin));

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies "), policies.size());
    for (size_t i = 0; i < policies.size(); ++i)
      workUnit->setWorkUnit(i, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), policies[i], true));
    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(true);
    context.run(workUnit);

    /*
    ** Optimize policy
    */
    FunctionPtr perception = new DiscreteBanditPerception(maxTimeStep);
    if (!perception->initialize(context, positiveIntegerType, banditStatisticsClass))
      return false;
    EnumerationPtr parametersEnumeration = DoubleVector::getElementsEnumeration(perception->getOutputType());
    if (!parametersEnumeration)
      return false;
    context.informationCallback(String((int)parametersEnumeration->getNumElements()) + T(" parameters"));
    context.resultCallback(T("parametersEnumeration"), parametersEnumeration);

    // eda parameters
    size_t numIterations = 20;
    size_t populationSize = 100;
    size_t numBests = 10;

    // optimizer state
    IndependentDoubleVectorDistributionPtr distribution = new IndependentDoubleVectorDistribution(parametersEnumeration);
    for (size_t i = 0; i < parametersEnumeration->getNumElements(); ++i)
      distribution->setSubDistribution(i, new GaussianDistribution(0.0, 1.0));
    OptimizerStatePtr optimizerState = new OptimizerState();
    optimizerState->setDistribution(context, distribution);

    // optimizer context
    FunctionPtr objectiveFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(perception, numBandits, maxTimeStep, trainingStates);
    objectiveFunction->initialize(context, distribution->getElementsType());
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(objectiveFunction);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests);
    optimizer->compute(context, optimizerContext, optimizerState);

    // best parameters
    DenseDoubleVectorPtr bestParameters = optimizerState->getBestVariable().getObjectAndCast<DenseDoubleVector>();
    context.resultCallback(T("bestParameters"), bestParameters);

    // evaluate on train and on test
    workUnit = new CompositeWorkUnit(T("Evaluating optimized policy"), 2);
    workUnit->setWorkUnit(0, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), new OptimizedDiscreteBanditPolicy(perception, bestParameters), true));
    workUnit->setWorkUnit(1, makeEvaluationWorkUnit(trainingStates, T("Training Problems"), new OptimizedDiscreteBanditPolicy(perception, bestParameters), true));
    workUnit->setProgressionUnit(T("Problem Sets"));
    workUnit->setPushChildrenIntoStackFlag(true);
    context.run(workUnit);
    
    // detailed evaluation
    DiscreteBanditPolicyPtr baselinePolicy = new UCB1TunedDiscreteBanditPolicy();
    DiscreteBanditPolicyPtr optimizedPolicy = new OptimizedDiscreteBanditPolicy(perception, bestParameters);
    context.enterScope(T("DetailedEvaluation"));
    double baselineScoreSum = 0.0;
    double optimizedScoreSum = 0.0;
    for (size_t i = 0; i < testingStates.size(); ++i)
    {
      context.enterScope(T("Problem ") + String((int)i));
      DiscreteBanditStatePtr initialState = testingStates[i];
      context.resultCallback(T("index"), i);
      context.resultCallback(T("problem"), initialState);
      std::vector<DiscreteBanditStatePtr> initialStates(1, initialState);

      double baselineScore = context.run(makeEvaluationWorkUnit(initialStates, T("Testing Problem"), baselinePolicy, true)).toDouble();
      baselineScoreSum += baselineScore;
      double optimizedScore = context.run(makeEvaluationWorkUnit(initialStates, T("Testing Problem"), optimizedPolicy, true)).toDouble();
      optimizedScoreSum += optimizedScore;

      context.resultCallback(T("baseline"), baselineScore);
      context.resultCallback(T("optimized"), optimizedScore);
      context.resultCallback(T("delta"), optimizedScore - baselineScore);
      context.leaveScope(new Pair(baselineScore, optimizedScore));
    }
    context.leaveScope(new Pair(baselineScoreSum / testingStates.size(), optimizedScoreSum / testingStates.size()));
    return true;
  }

protected:
  friend class BanditsSandBoxClass;

  size_t numBandits;
  size_t maxTimeStep;

  size_t numTrainingProblems;
  size_t numTestingProblems;

  WorkUnitPtr makeEvaluationWorkUnit(const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy, bool verbose) const
    {return new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, initialStatesDescription, policy, verbose);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
