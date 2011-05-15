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
# include <lbcpp/Sampler/Sampler.h>

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
  {
    statistics->push(reward);
    if (reward == 0)
    {
      for (size_t i = 0; i < numMoments; ++i)
        moments[i].push(0.0);
    }
    else if (reward == 1)
    {
      for (size_t i = 0; i < numMoments; ++i)
        moments[i].push(1.0);
    }
    else
      for (size_t i = 0; i < numMoments; ++i)
        moments[i].push(pow(reward, (double)(i + 2)));
  }

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

  size_t getNumMoments() const
    {return numMoments + 2;}

  // 0 -> 1
  // 1 -> mean
  // i -> E[X^i]^{1/i}
  double getMoment(size_t index) const
  {
    jassert(index < getNumMoments());
    if (index == 0)
      return 1.0;
    else if (index == 1)
      return statistics->getMean();
    else
      return pow(moments[index - 2].getMean(), 1.0 / (double)index);
  }

private:
  friend class BanditStatisticsClass;

  ScalarVariableStatisticsPtr statistics;
  enum {numMoments = 10};
  ScalarVariableMean moments[numMoments];
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

///////////////////////////////////////////////////
/////////// Optimized Bandit Policy ///////////////
///////////////////////////////////////////////////

class Parameterized
{
public:
  virtual ~Parameterized() {}

  virtual SamplerPtr createParametersSampler() const = 0;

  virtual void setParameters(const Variable& parameters) = 0;
  virtual Variable getParameters() const = 0;

  static Parameterized* get(const ObjectPtr& object)
    {return dynamic_cast<Parameterized* >(object.get());}

  static TypePtr getParametersType(const ObjectPtr& object)
    {Parameterized* p = get(object); return p ? p->getParameters().getType() : TypePtr();}

  static ObjectPtr cloneWithNewParameters(const ObjectPtr& object, const Variable& newParameters)
  {
    ObjectPtr res = object->clone(defaultExecutionContext());
    Parameterized* parameterized = get(res);
    jassert(parameterized);
    parameterized->setParameters(newParameters);
    return res;
  }
};

class EpsilonGreedyDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
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

  virtual SamplerPtr createParametersSampler() const
    {return objectCompositeSampler(pairClass(doubleType, doubleType), gaussianSampler(0.5, 0.5), gaussianSampler(0.5, 0.5));}

  virtual void setParameters(const Variable& parameters)
  {
    const PairPtr& pair = parameters.getObjectAndCast<Pair>();
    c = pair->getFirst().getDouble();
    d = pair->getSecond().getDouble();
  }

  virtual Variable getParameters() const
    {return new Pair(c, d);}

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

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    IndexBasedDiscreteBanditPolicy::clone(context, target);
    target.staticCast<EpsilonGreedyDiscreteBanditPolicy>()->random = random->cloneAndCast<RandomGenerator>();
  }

protected:
  friend class EpsilonGreedyDiscreteBanditPolicyClass;

  double c;
  double d;
  RandomGeneratorPtr random;
};

class WeightedUCBBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  WeightedUCBBanditPolicy(double C = 1.0)
    : C(C) {}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(1.0, 1.0);}

  virtual void setParameters(const Variable& parameters)
    {C = parameters.toDouble();}

  virtual Variable getParameters() const
    {return C;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + C * sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }

protected:
  friend class WeightedUCBBanditPolicyClass;

  double C;
};

extern EnumerationPtr timeDependentWeightedUCBBanditPolicyEnumerationEnumeration;

class TimeDependentWeightedUCBBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  TimeDependentWeightedUCBBanditPolicy()
    : parameters(new DenseDoubleVector(timeDependentWeightedUCBBanditPolicyEnumerationEnumeration, doubleType)) {}

  virtual SamplerPtr createParametersSampler() const
    {return independentDoubleVectorSampler(parameters->getElementsEnumeration(), gaussianSampler(0.0, 1.0));}

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
    double a = parameters->getValue(0);
    double b = parameters->getValue(1);
    double c = parameters->getValue(2);

    double C = a * log10((double)timeStep) + b * bandit->getPlayedCount() + c;
    return bandit->getRewardMean() + sqrt(2 * C * log((double)timeStep) / bandit->getPlayedCount());
  }

protected:
  friend class TimeDependentWeightedUCBBanditPolicyClass;
  // score = a * log((double)timeStep) + b * statistics->getPlayedCount() + c
  DenseDoubleVectorPtr parameters;
};

class PerTimesWeightedUCBBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  PerTimesWeightedUCBBanditPolicy(size_t maxTimeStep)
    : maxTimeStep(maxTimeStep), matrix(new DoubleMatrix(maxTimeStep, maxTimeStep, 1.0)) {}
  PerTimesWeightedUCBBanditPolicy() {}

  virtual SamplerPtr createParametersSampler() const
    {return independentDoubleMatrixSampler(maxTimeStep, maxTimeStep, gaussianSampler(1.0, 1.0));}

  virtual void setParameters(const Variable& parameters)
    {matrix = parameters.getObjectAndCast<DoubleMatrix>();}

  virtual Variable getParameters() const
    {return matrix;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    jassert(timeStep <= maxTimeStep);
    jassert(statistics->getPlayedCount() < maxTimeStep);
    return statistics->getRewardMean() + matrix->getValue(timeStep - 1, statistics->getPlayedCount()) * sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }

protected:
  friend class PerTimesWeightedUCBBanditPolicyClass;

  size_t maxTimeStep;
  DoubleMatrixPtr matrix;
};

class MomentsWeightedUCBBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  MomentsWeightedUCBBanditPolicy(size_t numMoments)
  {
    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("moments"));
    parametersEnumeration->addElement(defaultExecutionContext(), T("1"));
    for (size_t i = 1; i < numMoments; ++i)
      parametersEnumeration->addElement(defaultExecutionContext(), T("E[r^") + String((int)i) + T("]"));
    parameters = new DenseDoubleVector(parametersEnumeration, doubleType);
  }
  MomentsWeightedUCBBanditPolicy() {}

  virtual SamplerPtr createParametersSampler() const
    {return independentDoubleVectorSampler(parameters->getElementsEnumeration(), gaussianSampler(0.0, 1.0));}

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
    double C = 0.0;
    for (size_t i = 0; i < parameters->getNumElements(); ++i)
      C += bandit->getMoment(i) * parameters->getValue(i);
    return bandit->getRewardMean() + sqrt(C * log((double)timeStep) / bandit->getPlayedCount());
  }

protected:
  friend class MomentsWeightedUCBBanditPolicyClass;

  DenseDoubleVectorPtr parameters;
};

extern EnumerationPtr oldStyleParameterizedBanditPolicyEnumerationEnumeration;
class OldStyleParameterizedBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  OldStyleParameterizedBanditPolicy()
    : parameters(new DenseDoubleVector(oldStyleParameterizedBanditPolicyEnumerationEnumeration, doubleType))
  {
  }

  virtual SamplerPtr createParametersSampler() const
    {return independentDoubleVectorSampler(parameters->getElementsEnumeration(), gaussianSampler(0.0, 1.0));}

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
    
    double ts = log((double)timeStep);
    double mean = bandit->getRewardMean();
    double stddev = bandit->getRewardStandardDeviation();
    double conf = sqrt(log((double)timeStep) / bandit->getPlayedCount());

    double res = 0.0;
    size_t index = 0;
    
    res += mean * parameters->getValue(index++);
    res += stddev * parameters->getValue(index++);
    res += conf * parameters->getValue(index++);

    res += ts * mean * parameters->getValue(index++);
    res += ts * stddev * parameters->getValue(index++);
    res += ts * conf * parameters->getValue(index++);

    res += mean * mean * parameters->getValue(index++);
    res += mean * stddev * parameters->getValue(index++);
    res += mean * conf * parameters->getValue(index++);

    res += stddev * stddev * parameters->getValue(index++);
    res += stddev * conf * parameters->getValue(index++);
    res += conf * conf * parameters->getValue(index++);

    return res;
  }

protected:
  friend class OldStyleParameterizedBanditPolicyClass;

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
    size_t batchSize = 2;//numBandits;
    size_t timeStep = batchSize;
    size_t batchIndex = 0;
    while (timeStep < maxTimeStep)
    {
      timeSteps.push_back(timeStep);
      ++batchIndex;
      batchSize *= 2;//numBandits;
      timeStep += batchSize;
    }
    
    // initial accumulators
    std::vector<double> actualRegretVector(timeSteps.size(), 0.0);
    std::vector<double> bestMachinePlayedVector(timeSteps.size(), 0.0);

    ScalarVariableStatisticsPtr actualRegretStatistics = new ScalarVariableStatistics(T("actualRegret")); 

    // /!\ should be an argument of the WorkUnit
    size_t numEstimationsPerBandit = 100;

    // main calculation loop
    for (size_t i = 0; i < initialStates.size(); ++i)
    {
      for (size_t estimation = 0; estimation < numEstimationsPerBandit; ++estimation)
      {
        DiscreteBanditStatePtr state = initialStates[i]->cloneAndCast<DiscreteBanditState>();
  
        static int globalSeed = 1664;
        state->setSeed((juce::uint32)globalSeed);
        juce::atomicIncrement(globalSeed);
        
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
      }
      if (verbose)
        context.progressCallback(new ProgressionState(i + 1, initialStates.size(), T("Problems")));
    }

    double invZ = 1.0 / (initialStates.size() * numEstimationsPerBandit);
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
  EvaluateOptimizedDiscreteBanditPolicyParameters(const DiscreteBanditPolicyPtr& policy, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates)
    : SimpleUnaryFunction(Parameterized::getParametersType(policy), doubleType), policy(policy),
      numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), verbose(false)
  {
  }
  EvaluateOptimizedDiscreteBanditPolicyParameters() : SimpleUnaryFunction(variableType, variableType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DiscreteBanditPolicyPtr policy = Parameterized::cloneWithNewParameters(this->policy, input);
    WorkUnitPtr workUnit = new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Training Problems"), policy, verbose);
    ScalarVariableStatisticsPtr stats = context.run(workUnit, false).getObjectAndCast<ScalarVariableStatistics>();
    return stats->getMean();
    //return stats->getMaximum();
  }

protected:
  friend class EvaluateOptimizedDiscreteBanditPolicyParametersClass;

  DiscreteBanditPolicyPtr policy;
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
  BanditsSandBox() : numBandits(2), maxTimeStep(100000), numTrainingProblems(100), numTestingProblems(1000) {}
 
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
     // if ((i % 10) == 0)
        for (size_t j = 0; j < probs.size(); ++j)
          probs[j] = random->sampleDouble();
      //std::random_shuffle(probs.begin(), probs.end());
      trainingStates[i] = new BernouilliDiscreteBanditState(probs, random->sampleUint32());
    }

    std::vector<DiscreteBanditStatePtr> testingStates(numTestingProblems);
    for (size_t i = 0; i < testingStates.size(); ++i)
    {
      //if ((i % 10) == 0)
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

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies "), policies.size());
    for (size_t i = 0; i < policies.size(); ++i)
      workUnit->setWorkUnit(i, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), policies[i], true));
    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(true);
    context.run(workUnit);

    std::vector<DiscreteBanditPolicyPtr> policiesToOptimize;
    //policiesToOptimize.push_back(new PerTimesWeightedUCBBanditPolicy(maxTimeStep));
    //policiesToOptimize.push_back(new WeightedUCBBanditPolicy());
    policiesToOptimize.push_back(new OldStyleParameterizedBanditPolicy());
    policiesToOptimize.push_back(new TimeDependentWeightedUCBBanditPolicy());
    policiesToOptimize.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.1, 0.1));

    for (size_t i = 1; i < 10; ++i)
      policiesToOptimize.push_back(new MomentsWeightedUCBBanditPolicy(i));

    for (size_t i = 0; i < policiesToOptimize.size(); ++i)
    {
      DiscreteBanditPolicyPtr policyToOptimize = policiesToOptimize[i];
      context.enterScope(T("Optimizing ") + policyToOptimize->toString());
      Variable bestParameters = optimizePolicy(context, policyToOptimize, trainingStates, 20);
      DiscreteBanditPolicyPtr optimizedPolicy = Parameterized::cloneWithNewParameters(policyToOptimize, bestParameters);

      // evaluate on train and on test
      workUnit = new CompositeWorkUnit(T("Evaluating optimized policy"), 2);
      // policy must be cloned since it may be used in multiple threads simultaneously
      workUnit->setWorkUnit(0, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true));
      workUnit->setWorkUnit(1, makeEvaluationWorkUnit(trainingStates, T("Training Problems"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true));
      workUnit->setProgressionUnit(T("Problem Sets"));
      workUnit->setPushChildrenIntoStackFlag(true);
      context.run(workUnit);
      
      // detailed evaluation
      DiscreteBanditPolicyPtr baselinePolicy = new UCB1TunedDiscreteBanditPolicy();
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
      PairPtr result = new Pair(baselineScoreSum / testingStates.size(), optimizedScoreSum / testingStates.size());
      context.leaveScope(result);
      context.leaveScope(result);
    }
    return true;
  }

protected:
  friend class BanditsSandBoxClass;

  size_t numBandits;
  size_t maxTimeStep;

  size_t numTrainingProblems;
  size_t numTestingProblems;

//  DiscreteBanditPolicyPtr policyToOptimize;

  WorkUnitPtr makeEvaluationWorkUnit(const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy, bool verbose) const
    {return new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, initialStatesDescription, policy, verbose);}
  
  Variable optimizePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& trainingStates, size_t numIterations = 20)
  {
    TypePtr parametersType = Parameterized::getParametersType(policy);
    jassert(parametersType);

    context.resultCallback(T("parametersType"), parametersType);

    // eda parameters
    size_t populationSize = 100;
    size_t numBests = 10;

    // optimizer state
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(Parameterized::get(policy)->createParametersSampler());

    // optimizer context
    FunctionPtr objectiveFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, maxTimeStep, trainingStates);
    objectiveFunction->initialize(context, parametersType);
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objectiveFunction);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests);
    //OptimizerPtr optimizer = asyncEDAOptimizer(numIterations*populationSize, populationSize, populationSize/numBests, 1, 10, populationSize);

    optimizer->compute(context, optimizerContext, optimizerState);

    // best parameters
    Variable bestParameters = optimizerState->getBestVariable();
    context.resultCallback(T("bestParameters"), bestParameters);
    return bestParameters;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
