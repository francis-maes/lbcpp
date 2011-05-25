/*-----------------------------------------.---------------------------------.
| Filename: DiscreteBanditExperiment.h     | Discrete Bandits Experiment     |
| Author  : Francis Maes                   |                                 |
| Started : 24/05/2011 21:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_EXPERIMENT_H_
# define LBCPP_BANDITS_DISCRETE_EXPERIMENT_H_

# include "Bandits/DiscreteBanditPolicy.h"
# include "../GP/GPExpression.h"

namespace lbcpp
{

/*
** Evaluate functions
*/
class EvaluateDiscreteBanditPolicyWorkUnit : public WorkUnit
{
public:
  EvaluateDiscreteBanditPolicyWorkUnit(size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy,  size_t numEstimationsPerBandit = 100, bool verbose = true)
    : numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), initialStatesDescription(initialStatesDescription), policy(policy), numEstimationsPerBandit(numEstimationsPerBandit), verbose(verbose) {}

  EvaluateDiscreteBanditPolicyWorkUnit()
    : numBandits(0), maxTimeStep(0), verbose(false) {}

  virtual String toShortString() const
    {return T("Evaluate ") + policy->toShortString() + T(" on ") + initialStatesDescription;}

  virtual Variable run(ExecutionContext& context)
  {
    double log10MaxTimeStep = log10((double)maxTimeStep);
    size_t numPowersOfTen = (size_t)log10MaxTimeStep;

    // initial accumulators
    DenseDoubleVectorPtr actualRegretVector = new DenseDoubleVector(numPowersOfTen + (log10MaxTimeStep > numPowersOfTen ? 1 : 0), 0.0);

    // main calculation loop
    context.getRandomGenerator()->setSeed(16645186); // make the function deterministic

    for (size_t i = 0; i < initialStates.size(); ++i)
    {
      DiscreteBanditStatePtr state = initialStates[i];
      std::vector<double> expectedRewards(numBandits);
      double bestReward = -DBL_MAX;
      size_t optimalBandit = 0;
      for (size_t j = 0; j < numBandits; ++j)
      {
        double er = state->getExpectedReward(j);
        expectedRewards[j] = er;
        if (er > bestReward)
          bestReward = er, optimalBandit = j;
      }

      for (size_t estimation = 0; estimation < numEstimationsPerBandit; ++estimation)
      {
        policy->initialize(numBandits);

        double sumOfRewards = 0.0;

        size_t index = 0;
        for (size_t timeStep = 1; timeStep <= maxTimeStep; ++timeStep)
        {
          size_t action = performBanditStep(context, state, policy);
          sumOfRewards += expectedRewards[action];

          double lt = log10((double)timeStep);
          if (timeStep >= 10 && (lt == (double)((int)lt) || timeStep == maxTimeStep))
            actualRegretVector->incrementValue(index++, timeStep * bestReward - sumOfRewards);
        }
      }
      if (verbose)
        context.progressCallback(new ProgressionState(i + 1, initialStates.size(), T("Problems")));
    }

    actualRegretVector->multiplyByScalar(1.0 / (initialStates.size() * numEstimationsPerBandit));

    if (verbose)
      for (size_t i = 0; i < numPowersOfTen; ++i)
      {
      //bestMachinePlayedVector[i] *= invZ;
        context.resultCallback(T("actualRegret@") + String(pow(10.0, i + 1.0)), actualRegretVector->getValue(i));
/*
      if (verbose)
      {
        size_t timeStep = timeSteps[i];
        context.enterScope(String((int)timeStep) + T(" Steps"));
        context.resultCallback(T("log10(n)"), log10((double)timeStep));
        //context.resultCallback(T("bestMachinePlayed"), bestMachinePlayedVector[i] * 100.0);
        context.resultCallback(T("actualRegret"), actualRegretVector[i]);
        context.resultCallback(T("n"), timeStep);
        context.leaveScope(actualRegretVector[i]);
      }*/
      }

    // TMP !!!
    //context.resultCallback(T("policy"), policy);
    //context.resultCallback(T("log10(C)"), log10(policy->getVariable(0).toDouble()));
    //context.resultCallback(T("score"), actualRegretStatistics->getMean());

    return actualRegretVector;
  }
 
protected:
  friend class EvaluateDiscreteBanditPolicyWorkUnitClass;

  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  String initialStatesDescription;
  DiscreteBanditPolicyPtr policy;
  size_t numEstimationsPerBandit;
  bool verbose;

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }
};

class EvaluateOptimizedDiscreteBanditPolicyParameters : public SimpleUnaryFunction
{
public:
  EvaluateOptimizedDiscreteBanditPolicyParameters(const DiscreteBanditPolicyPtr& policy, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates, size_t numEstimationsPerBandit = 100, double l0Weight = 0.0)
    : SimpleUnaryFunction(Parameterized::getParametersType(policy), doubleType), policy(policy),
      numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), numEstimationsPerBandit(numEstimationsPerBandit), verbose(false), l0Weight(l0Weight)
  {
  }
  EvaluateOptimizedDiscreteBanditPolicyParameters() : SimpleUnaryFunction(variableType, variableType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DiscreteBanditPolicyPtr policy = Parameterized::cloneWithNewParameters(this->policy, input);
    WorkUnitPtr workUnit = new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Training Problems"), policy, numEstimationsPerBandit, verbose);
    DenseDoubleVectorPtr regrets = context.run(workUnit, false).getObjectAndCast<DenseDoubleVector>();
    double res = regrets->getValue(regrets->getNumValues() - 1);;
    if (l0Weight)
    {
      const DenseDoubleVectorPtr& inputVector = input.getObjectAndCast<DenseDoubleVector>();
      res += inputVector->l0norm() * l0Weight;
    }
    return res;
  }

protected:
  friend class EvaluateOptimizedDiscreteBanditPolicyParametersClass;

  DiscreteBanditPolicyPtr policy;
  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  size_t numEstimationsPerBandit;
  bool verbose;
  double l0Weight;
};

///////////////////////////////////////////////////////////////


class DiscreteBanditExperiment : public WorkUnit
{
public:
  DiscreteBanditExperiment() : numBandits(2), numTrainingProblems(100), numTestingProblems(10000), maxTimeStep(1000) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    sampleProblems(context, trainingProblems, testingProblems, generalizationProblems);
    bool ok;

    /*
    for (double C = 0.0; C <= 2.0; C += 0.1)
    {
      context.enterScope(T("C = ") + String(C));
      context.resultCallback(T("C"), C);
      evaluatePolicy(context, ucb1DiscreteBanditPolicy(C), false);
      context.leaveScope(true);
    }
    return true;*/

/*    context.enterScope(T("Untuned policies"));
    ok = evaluateUntunedPolicies(context);
    context.leaveScope(ok);  
*/
    context.enterScope(T("Tuned policies"));
    ok = tuneAndEvaluatePolicies(context);
    context.leaveScope(ok);
/*
    context.enterScope(T("Learned policies"));
    ok = learnAndEvaluatePolicies(context);
    context.leaveScope(ok);
*/
    return true;
  }

protected:
  friend class DiscreteBanditExperimentClass;
 
  size_t numBandits;
  size_t numTrainingProblems;
  size_t numTestingProblems;
  size_t maxTimeStep;

  std::vector<DiscreteBanditStatePtr> trainingProblems;
  std::vector<DiscreteBanditStatePtr> testingProblems;
  std::vector<DiscreteBanditStatePtr> generalizationProblems;

  void sampleProblems(ExecutionContext& context, std::vector<DiscreteBanditStatePtr>& training, std::vector<DiscreteBanditStatePtr>& testing, std::vector<DiscreteBanditStatePtr>& generalization)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    /*
    ** Make training and testing problems
    */
    ClassPtr bernoulliSamplerClass = lbcpp::getType(T("BernoulliSampler"));
    ClassPtr gaussianSamplerClass = lbcpp::getType(T("GaussianSampler"));
    ClassPtr rejectionSamplerClass = lbcpp::getType(T("RejectionSampler"));

    SamplerPtr banditSamplerSampler = objectCompositeSampler(bernoulliSamplerClass, uniformScalarSampler(0.0, 1.0));
    SamplerPtr initialStateSampler = new DiscreteBanditInitialStateSampler(banditSamplerSampler, numBandits);
    SamplerPtr banditSamplerSampler2 = objectCompositeSampler(rejectionSamplerClass,
                                                objectCompositeSampler(gaussianSamplerClass, uniformScalarSampler(), uniformScalarSampler()),
                                                constantSampler(logicalAnd(lessThanOrEqualToPredicate(1.0), greaterThanOrEqualToPredicate(0.0))));
    SamplerPtr initialStateSampler2 = new DiscreteBanditInitialStateSampler(banditSamplerSampler2, numBandits);

    training.resize(numTrainingProblems);
    for (size_t i = 0; i < training.size(); ++i)
      training[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    testing.resize(numTestingProblems);
    for (size_t i = 0; i < testing.size(); ++i)
      testing[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    generalization.resize(numTestingProblems);
    for (size_t i = 0; i < generalization.size(); ++i)
      generalization[i] = initialStateSampler2->sample(context, random).getObjectAndCast<DiscreteBanditState>();
  }

  bool evaluateUntunedPolicies(ExecutionContext& context) const
  {
    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(greedyDiscreteBanditPolicy());
    policies.push_back(ucb1DiscreteBanditPolicy());
    policies.push_back(ucb1TunedDiscreteBanditPolicy());
    policies.push_back(ucb1NormalDiscreteBanditPolicy());
    policies.push_back(ucb2DiscreteBanditPolicy());
    policies.push_back(ucbvDiscreteBanditPolicy());
    policies.push_back(epsilonGreedyDiscreteBanditPolicy());
    return evaluatePolicies(context, policies);
  }

  bool tuneAndEvaluatePolicies(ExecutionContext& context) const
  {
    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(ucb1DiscreteBanditPolicy());
    policies.push_back(ucb2DiscreteBanditPolicy());
    policies.push_back(ucbvDiscreteBanditPolicy());
    policies.push_back(epsilonGreedyDiscreteBanditPolicy());
    return optimizeAndEvaluatePolicies(context, policies);
  }

  bool learnAndEvaluatePolicies(ExecutionContext& context) const
  {
    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(powerDiscreteBanditPolicy(1, false));
    policies.push_back(powerDiscreteBanditPolicy(2, false));
    return optimizeAndEvaluatePolicies(context, policies);
  }

private:
  bool optimizeAndEvaluatePolicies(ExecutionContext& context, std::vector<DiscreteBanditPolicyPtr>& policies) const
  {
    bool allOk = true;
    for (size_t horizon = 10; horizon <= maxTimeStep; horizon *= 10)
    {
      context.enterScope(T("Horizon ") + String((int)horizon));
      std::vector<DiscreteBanditPolicyPtr> pols(policies.size());
      for (size_t i = 0; i < pols.size(); ++i)
        pols[i] = policies[i]->cloneAndCast<DiscreteBanditPolicy>(context);
      bool ok = optimizePolicies(context, pols, horizon) && evaluatePolicies(context, pols, false);
      context.leaveScope(ok);
      allOk &= ok;
    }
    return allOk;
  }

  bool optimizePolicies(ExecutionContext& context, std::vector<DiscreteBanditPolicyPtr>& policies, size_t horizon) const
  {
    bool allOk = true;
    for (size_t i = 0; i < policies.size(); ++i)
    {
      DiscreteBanditPolicyPtr& policy = policies[i];
      context.enterScope(T("Optimize policy ") + policy->toShortString());
      bool ok = optimizePolicy(context, policy, horizon);
      allOk &= ok;
      context.leaveScope(ok);
    }
    return allOk;
  }

  bool optimizePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr& policy, size_t horizon) const
  {
    TypePtr parametersType = Parameterized::getParametersType(policy);
    jassert(parametersType);
    size_t numParameters = 0;
    EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(parametersType);
    if (enumeration)
      numParameters = enumeration->getNumElements();
    else if (parametersType->inheritsFrom(doubleType))
      numParameters = 1;
    else if (parametersType->inheritsFrom(pairClass(doubleType, doubleType)))
      numParameters = 2;
    else if (parametersType->inheritsFrom(gpExpressionClass))
      numParameters = 100;
    jassert(numParameters);
    context.resultCallback(T("numParameters"), numParameters);

    // eda parameters
    size_t numIterations = (numParameters <= 2 ? 30 : 100);
    size_t populationSize = numParameters * 8;
    size_t numBests = numParameters * 2;

    if (populationSize < 50)
      populationSize = 50;
    if (numBests < 10)
      numBests = 10;

    // optimizer state
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(Parameterized::get(policy)->createParametersSampler());

    // optimizer context
    FunctionPtr objectiveFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, horizon, trainingProblems, 100);
    objectiveFunction->initialize(context, parametersType);

    FunctionPtr validationFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, horizon, testingProblems, 1);
    validationFunction->initialize(context, parametersType);

    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objectiveFunction, validationFunction);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, 0, true);
    optimizer->compute(context, optimizerContext, optimizerState);

    // best parameters
    Variable bestParameters = optimizerState->getBestVariable();
    policy = Parameterized::cloneWithNewParameters(policy, bestParameters);
    context.informationCallback(policy->toShortString());
    context.resultCallback(T("optimizedPolicy"), policy);
    return policy;
  }

  bool evaluatePolicy(ExecutionContext& context, const DiscreteBanditPolicyPtr& policy, bool pushIntoStack = false) const
    {return evaluatePolicies(context, std::vector<DiscreteBanditPolicyPtr>(1, policy), pushIntoStack);}

  bool evaluatePolicies(ExecutionContext& context, const std::vector<DiscreteBanditPolicyPtr>& policies, bool pushIntoStack = false) const
  {
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies"), policies.size());
    for (size_t i = 0; i < policies.size(); ++i)
      workUnit->setWorkUnit(i, new EvaluatePolicyWorkUnit(policies[i], numBandits, maxTimeStep, testingProblems, generalizationProblems));
    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(policies.size() > 1);
    context.run(workUnit, pushIntoStack);
    return true;
  }

  struct EvaluatePolicyWorkUnit : public WorkUnit
  {
    EvaluatePolicyWorkUnit(const DiscreteBanditPolicyPtr& policy, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& testingProblems,
                                                                  const std::vector<DiscreteBanditStatePtr>& generalizationProblems)
        : policy(policy), numBandits(numBandits), maxTimeStep(maxTimeStep), testingProblems(testingProblems), generalizationProblems(generalizationProblems) {}
              
    virtual Variable run(ExecutionContext& context)
    {
      WorkUnitPtr workUnit = new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, testingProblems, T("Testing Problems"), policy, 100, true);
      DenseDoubleVectorPtr testingResult = context.run(workUnit).getObjectAndCast<DenseDoubleVector>();

      for (size_t i = 0; i < testingResult->getNumElements(); ++i)
        context.resultCallback(T("testingRegret@") + String(pow(10.0, i + 1.0)), testingResult->getValue(i));

      workUnit = new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, generalizationProblems, T("Generalization Problems"), policy, 100, true);
      DenseDoubleVectorPtr generalizationResult = context.run(workUnit).getObjectAndCast<DenseDoubleVector>();

      for (size_t i = 0; i < generalizationResult->getNumElements(); ++i)
        context.resultCallback(T("generalizationRegret@") + String(pow(10.0, i + 1.0)), generalizationResult->getValue(i));

      PairPtr res(new Pair(testingResult, generalizationResult));
      context.informationCallback(res->toShortString());
      return res;
    }
    
    virtual String toShortString() const
      {return T("Evaluating policy ") + policy->toShortString();}

  protected:
    DiscreteBanditPolicyPtr policy;
    size_t numBandits;
    size_t maxTimeStep;
    const std::vector<DiscreteBanditStatePtr>& testingProblems;
    const std::vector<DiscreteBanditStatePtr>& generalizationProblems;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_EXPERIMENT_H_
