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

class SmallMDPSandBox : public WorkUnit
{
public:
  SmallMDPSandBox() : mdpSampler(new SparseSmallMDPSampler()), numTimeSteps(100000), numRuns(1000) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    testPolicy(context, "optimal", new OptimalSmallMDPPolicy());
    testPolicy(context, "random", new RandomSmallMDPPolicy());

    std::vector<SmallMDPPolicyPtr> policies;
    for (size_t m = 1; m < 50; ++m)
      policies.push_back(new RMaxSmallMDPPolicy(m));
    findBestPolicy(context, "rmax", policies);

    policies.clear();
    for (size_t m = 1; m < 50; ++m)
      policies.push_back(new RTDPRMaxSmallMDPPolicy(m));
    findBestPolicy(context, "RTDP-rmax", policies);
    
    policies.clear();
    for (double beta = 0.0; beta <= 2.0; beta += 0.1)
      policies.push_back(new QLearningSmallMDPPolicy(constantIterationFunction(0.0), beta));
    findBestPolicy(context, "QLearning", policies);

    /*
    //SmallMDPPtr mdp = new LongChainMDP();
    //SmallMDPPtr mdp = new BanditMDP();
    //testPolicy(context, "MBIE-EB(10000, 0.16)", new MBIEEBSmallMDPPolicy(10000, 0.16), mdp);
    
    testPolicy(context, "rmax(2)", new RMaxSmallMDPPolicy(2));
    testPolicy(context, "RTDP-rmax(2)", new RTDPRMaxSmallMDPPolicy(2));
//    testPolicy(context, "rmax(5)", new RMaxSmallMDPPolicy(5), mdp);
    //testPolicy(context, "rmax(10)", new RMaxSmallMDPPolicy(10), mdp);
    testPolicy(context, "qlearning w=1", new QLearningSmallMDPPolicy(constantIterationFunction(0.0), 1.0));   
    //testPolicy(context, "qlearning w=0.5", new QLearningSmallMDPPolicy(constantIterationFunction(0.0), 0.5), mdp);   
    testPolicy(context, "random", new RandomSmallMDPPolicy());
    */
    return true;
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
    return bestPolicy;
  }

  double testPolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy) const
  {
    context.enterScope(name);

    for (size_t i = 0; i < policy->getNumVariables(); ++i)
      if (policy->getVariableType(i)->isConvertibleToDouble())
        context.resultCallback(policy->getVariableName(i), policy->getVariable(i));

    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("toto");
    for (size_t i = 0; i < numRuns; ++i)
    {
      SmallMDPPtr mdp = mdpSampler->sample(context, context.getRandomGenerator()).getObjectAndCast<SmallMDP>();
      double score = runPolicy(context, policy->cloneAndCast<SmallMDPPolicy>(), mdp, numTimeSteps);
      stats->push(score);
      context.progressCallback(new ProgressionState(i+1, numRuns, T("Runs")));
      context.informationCallback("Score: " + String(score));
    }

    context.resultCallback("mean cumulative reward", stats->getMean());
    context.leaveScope(stats);
    return stats->getMean();
  }
  
  double runPolicy(ExecutionContext& context, const SmallMDPPolicyPtr& policy, const SmallMDPPtr& mdp, size_t numTimeSteps) const
  {
    policy->initialize(context, mdp);
    double rewardSum = 0.0;
    size_t state = mdp->getInitialState();
    for (size_t i = 0; i < numTimeSteps; ++i)
    {
      size_t action = policy->selectAction(context, state);
      double reward;
      size_t newState = mdp->sampleTransition(context, state, action, reward);
      policy->observeTransition(context, state, action, newState, reward);
      state = newState;
      rewardSum += reward;
    }
    return rewardSum;
  }

protected:
  friend class SmallMDPSandBoxClass;
  
  SamplerPtr mdpSampler;
  size_t numTimeSteps;
  size_t numRuns;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_SANDBOX_H_

