/*-----------------------------------------.---------------------------------.
| Filename: SmallMDPSandBox.h              | Small MDP Sand Box              |
| Author  : Francis Maes                   |                                 |
| Started : 07/11/2011 16:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SMALL_MDP_SANDBOX_H_
# define LBCPP_SMALL_MDP_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "SmallMDP.h"
# include "SmallMDPPolicy.h"
# include "ParameterizedSmallMDPPolicy.h"

namespace lbcpp
{

class SparseSmallMDPSampler : public Sampler
{
public:
  SparseSmallMDPSampler(size_t numStates = 20, size_t numActions = 4, double discount = 0.9, size_t numSuccessorsPerState = 5, double nonNullRewardProbability = 0.1)
    : numStates(numStates), numActions(numActions), discount(discount), numSuccessorsPerState(numSuccessorsPerState), nonNullRewardProbability(nonNullRewardProbability) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return new GeneratedSparseSmallMDP(random, numStates, numActions, discount, numSuccessorsPerState, nonNullRewardProbability);}
 
protected:
  friend class SparseSmallMDPSamplerClass;

  size_t numStates;
  size_t numActions;
  double discount;
  size_t numSuccessorsPerState;
  double nonNullRewardProbability;
};

class ConstantMDPSampler : public Sampler
{
public:
  ConstantMDPSampler(const SmallMDPPtr& mdp = SmallMDPPtr())
    : mdp(mdp) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return mdp;}
 
protected:
  friend class ConstantMDPSamplerClass;
  
  SmallMDPPtr mdp;
};

class EvaluateSmallMDPPolicy : public WorkUnit
{
public:
  EvaluateSmallMDPPolicy(const SmallMDPPolicyPtr& policy, const SmallMDPPtr& mdp)
    : policy(policy), mdp(mdp)
  {
  }
  EvaluateSmallMDPPolicy()  {}
    
  virtual Variable run(ExecutionContext& context)
  {
    double discountFactor = mdp->getDiscount();
    size_t numTimeSteps = (size_t)ceil(log(0.05 * (1 - discountFactor)) / log(discountFactor));

    SmallMDPPolicyPtr policy = this->policy->cloneAndCast<SmallMDPPolicy>();
    
    policy->initialize(context, mdp);
    size_t state = mdp->getInitialState();
    double discountedCumulativeReward = 0.0;
    double discount = 1.0;

    for (size_t i = 0; i < numTimeSteps; ++i)
    {
      size_t action = policy->selectAction(context, state);
      double reward;
      size_t newState = mdp->sampleTransition(context, state, action, reward);
      policy->observeTransition(context, state, action, newState, reward);
      state = newState;
      discountedCumulativeReward += discount * reward;
      discount *= discountFactor;
    }
    return discountedCumulativeReward;
  }
  
protected:
  SmallMDPPolicyPtr policy;
  SmallMDPPtr mdp;
};

class EvaluateSmallMDPPolicyCompositeWorkUnit : public CompositeWorkUnit
{
public:
  EvaluateSmallMDPPolicyCompositeWorkUnit(const SmallMDPPolicyPtr& policy, const SamplerPtr& mdpSampler, size_t numRuns)
    : CompositeWorkUnit("Evaluate " + policy->toShortString(), numRuns)
  {
    ExecutionContext& context = defaultExecutionContext();
    for (size_t i = 0; i < numRuns; ++i)
    {
      SmallMDPPtr mdp = mdpSampler->sample(context, context.getRandomGenerator()).getObjectAndCast<SmallMDP>();
      setWorkUnit(i, new EvaluateSmallMDPPolicy(policy, mdp));
    }
    setProgressionUnit("Runs");
  }
};

class EvaluateSmallMDPPolicyParameters : public SimpleUnaryFunction
{
public:
  EvaluateSmallMDPPolicyParameters(const SmallMDPPolicyPtr& policy, const SamplerPtr& mdpSampler, size_t numRuns)
    : SimpleUnaryFunction(Parameterized::getParametersType(policy), doubleType), policy(policy), mdpSampler(mdpSampler), numRuns(numRuns) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    SmallMDPPolicyPtr policy = Parameterized::cloneWithNewParameters(this->policy, input);
    VariableVectorPtr results = context.run(CompositeWorkUnitPtr(new EvaluateSmallMDPPolicyCompositeWorkUnit(policy, mdpSampler, numRuns)), false).getObjectAndCast<VariableVector>();
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("toto");
    for (size_t i = 0; i < numRuns; ++i)
      stats->push(results->getElement(i).getDouble());
    return -stats->getMean();
  }
  
protected:
  SmallMDPPolicyPtr policy;
  SamplerPtr mdpSampler;
  size_t numRuns;
};

class SmallMDPSandBox : public WorkUnit
{
public:
  SmallMDPSandBox() : mdpSampler(new SparseSmallMDPSampler()), numRuns(1000) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    std::vector<SmallMDPPolicyPtr> policies;

    SmallMDPPolicyPtr policy = new OptimalSmallMDPPolicy();
    testPolicy(context, "optimal", policy);
    savePolicy(context, "optimal", policy);

    policy = new RandomSmallMDPPolicy();
    testPolicy(context, "random", policy);
    savePolicy(context, "random", policy);

    policies.clear();
    for (double beta = 0.0; beta <= 3.0; beta += 0.1)
      policies.push_back(new QLearningSmallMDPPolicy(constantIterationFunction(0.0), beta));
    findBestPolicy(context, "QLearning", policies);

    policies.clear();
    for (size_t m = 1; m < 50; ++m)
      policies.push_back(new RMaxSmallMDPPolicy(m));
    findBestPolicy(context, "rmax", policies);

    policies.clear();
    for (size_t m = 1; m < 50; ++m)
      policies.push_back(new RTDPRMaxSmallMDPPolicy(m));
    findBestPolicy(context, "RTDP-rmax", policies);

    policy = new ParameterizedModelBasedSmallMDPPolicy(0);
    testPolicy(context, "FullMB-base1", policy);
    savePolicy(context, "FullMB-base1", policy);

    policy = new ParameterizedModelBasedSmallMDPPolicy(1);
    policy.staticCast<ParameterizedModelBasedSmallMDPPolicy>()->setParameter(0, -1.0);
    testPolicy(context, "FullMB-base2", policy);
    savePolicy(context, "FullMB-base2", policy);

    policy = new ParameterizedModelBasedSmallMDPPolicy(1);
    policy.staticCast<ParameterizedModelBasedSmallMDPPolicy>()->setParameter(0, -0.5);
    testPolicy(context, "FullMB-base3", policy);
    savePolicy(context, "FullMB-base3", policy);

    for (size_t i = 3; i <= 5; ++i)
      optimizePolicy(context, "parameterized-RTDP" + String((int)i), new ParameterizedSmallMDPPolicy(i*2, true));
      
    for (size_t i = 3; i <= 5; ++i)      
      optimizePolicy(context, "parameterized-QLearning" + String((int)i), new ParameterizedSmallMDPPolicy(i*2, false));

    for (size_t i = 1; i <= 4; ++i)
      optimizePolicy(context, "parameterized-FullMB" + String((int)i), new ParameterizedModelBasedSmallMDPPolicy(i));
    return true;
  }

  double testPolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy) const
  {
    context.enterScope(name);
    for (size_t i = 0; i < policy->getNumVariables(); ++i)
      if (policy->getVariableType(i)->isConvertibleToDouble())
        context.resultCallback(policy->getVariableName(i), policy->getVariable(i));
    
    CompositeWorkUnitPtr workUnit = new EvaluateSmallMDPPolicyCompositeWorkUnit(policy, mdpSampler, numRuns);
    VariableVectorPtr results = context.run(workUnit, false).getObjectAndCast<VariableVector>();

    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("toto");
    for (size_t i = 0; i < numRuns; ++i)
      stats->push(results->getElement(i).getDouble());
    context.resultCallback("mean cumulative reward", stats->getMean());
    context.leaveScope(stats);
    return stats->getMean();
  }
  
  SmallMDPPolicyPtr findBestPolicy(ExecutionContext& context, const String& name, const std::vector<SmallMDPPolicyPtr>& policies) const
  {
    double bestScore = -DBL_MAX;
    SmallMDPPolicyPtr bestPolicy;

    context.enterScope(name);
    for (size_t i = 0; i < policies.size(); ++i)
    {
      SmallMDPPolicyPtr policy = policies[i];
      double score = testPolicy(context, policy->toShortString(), policy);
      if (score > bestScore)
      {
        bestScore = score;
        bestPolicy = policy;
      }
    }
    context.leaveScope(bestScore);
    savePolicy(context, name, bestPolicy);
    return bestPolicy;
  }

  SmallMDPPolicyPtr optimizePolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy) const
  {
    context.enterScope(name);

    TypePtr parametersType = Parameterized::getParametersType(policy);
    jassert(parametersType);
    size_t numParameters = 0;
    EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(parametersType);
    if (enumeration)
      numParameters = enumeration->getNumElements();
    jassert(numParameters);
    context.resultCallback(T("numParameters"), numParameters);

    // eda parameters
    size_t numIterations = 10;
    size_t populationSize = numParameters * 8;
    size_t numBests = numParameters * 2;

    // optimization problem
    FunctionPtr objectiveFunction = new EvaluateSmallMDPPolicyParameters(policy, mdpSampler, numRuns);
    objectiveFunction->initialize(context, parametersType);
    OptimizationProblemPtr problem = new OptimizationProblem(objectiveFunction, Variable(), Parameterized::get(policy)->createParametersSampler());

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, StoppingCriterionPtr(), 0, true);
    OptimizerStatePtr state = optimizer->compute(context, problem).getObjectAndCast<OptimizerState>();

    // best parameters
    Variable bestParameters = state->getBestSolution();
    SmallMDPPolicyPtr optimizedPolicy = Parameterized::cloneWithNewParameters(policy, bestParameters);
    context.informationCallback(optimizedPolicy->toShortString());
    context.resultCallback(T("optimizedPolicy"), optimizedPolicy);

    savePolicy(context, name, optimizedPolicy);
    context.leaveScope(-state->getBestScore());
    return optimizedPolicy;
  }
  
  void savePolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy) const
  {
    if (outputDirectory != File::nonexistent)
    {
      if (!outputDirectory.exists())
        outputDirectory.createDirectory();
      policy->saveToFile(context, outputDirectory.getChildFile(name + T(".policy")));
    }
  }

protected:
  friend class SmallMDPSandBoxClass;
  
  SamplerPtr mdpSampler;
  size_t numRuns;
  File outputDirectory;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_SANDBOX_H_

