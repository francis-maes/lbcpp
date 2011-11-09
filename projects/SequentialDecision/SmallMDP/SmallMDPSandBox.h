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

class SmallMDPSandBox : public WorkUnit
{
public:
  SmallMDPSandBox() : numStates(20), numActions(4), discount(0.9), numSuccessorsPerState(5), nonNullRewardProbability(0.1) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    //SmallMDPPtr mdp = new GeneratedSparseSmallMDP(context.getRandomGenerator(), numStates, numActions, discount, numSuccessorsPerState, nonNullRewardProbability);

    SmallMDPPtr mdp = new LongChainMDP();
    //SmallMDPPtr mdp = new BanditMDP();
    testPolicy(context, "MBIE-EB(10000, 0.16)", new MBIEEBSmallMDPPolicy(10000, 0.16), mdp);
    testPolicy(context, "optimal", new OptimalSmallMDPPolicy(), mdp);
    testPolicy(context, "rmax(2)", new RMaxSmallMDPPolicy(2), mdp);
//    testPolicy(context, "rmax(5)", new RMaxSmallMDPPolicy(5), mdp);
    //testPolicy(context, "rmax(10)", new RMaxSmallMDPPolicy(10), mdp);
    testPolicy(context, "qlearning w=1", new QLearningSmallMDPPolicy(constantIterationFunction(0.0), 1.0), mdp);   
    //testPolicy(context, "qlearning w=0.5", new QLearningSmallMDPPolicy(constantIterationFunction(0.0), 0.5), mdp);   
    testPolicy(context, "random", new RandomSmallMDPPolicy(), mdp);
    return true;
  }

  void testPolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy, const SmallMDPPtr& mdp) const
  {
    static const size_t numTimeSteps = 100000;
    static const size_t numRuns = 10;

    context.enterScope(name);
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("toto");
    for (size_t i = 0; i < numRuns; ++i)
    {
      double score = runPolicy(context, policy->cloneAndCast<SmallMDPPolicy>(), mdp, numTimeSteps);
      stats->push(score);
      context.progressCallback(new ProgressionState(i+1, numRuns, T("Runs")));
      context.informationCallback("Score: " + String(score));
    }
    context.leaveScope(stats);
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
  
  size_t numStates;
  size_t numActions;
  double discount;
  size_t numSuccessorsPerState;
  double nonNullRewardProbability;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_SANDBOX_H_

