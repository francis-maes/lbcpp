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
  SmallMDPSandBox() : numStates(20), numActions(4), discount(0.9), numSuccessorsPerState(5) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    SmallMDPPtr mdp = sampleMDP(context.getRandomGenerator());
  
    testPolicy(context, "random", new RandomSmallMDPPolicy(), mdp);
    testPolicy(context, "optimal", new OptimalSmallMDPPolicy(), mdp);
    testPolicy(context, "qlearning", new QLearningSmallMDPPolicy(constantIterationFunction(0.1), 0.5), mdp);   

    testPolicy(context, "random", new RandomSmallMDPPolicy(), mdp);
    testPolicy(context, "optimal", new OptimalSmallMDPPolicy(), mdp);
    testPolicy(context, "qlearning", new QLearningSmallMDPPolicy(constantIterationFunction(0.1), 0.5), mdp);   
    return true;
  }

  void testPolicy(ExecutionContext& context, const String& name, const SmallMDPPolicyPtr& policy, const SmallMDPPtr& mdp) const
  {
    static const size_t numTimeSteps = 100000;

    context.enterScope(name);

    policy->initialize(context, mdp);
    double rewardSum = 0.0;
    size_t state = 0;
    for (size_t i = 0; i < numTimeSteps; ++i)
    {
      size_t action = policy->selectAction(context, state);
      double reward;
      size_t newState = mdp->sampleTransition(context, state, action, reward);
      policy->observeTransition(context, state, action, newState, reward);
      state = newState;
      rewardSum += reward;
    }
    
    context.leaveScope(rewardSum);
  }
  
protected:
  friend class SmallMDPSandBoxClass;
  
  size_t numStates;
  size_t numActions;
  double discount;
  size_t numSuccessorsPerState;
  
  SmallMDPPtr sampleMDP(RandomGeneratorPtr randomGenerator) const
  {
    SmallMDPPtr res(new SmallMDP(numStates, numActions, discount));
    for (size_t i = 0; i < numStates; ++i)
      for (size_t j = 0; j < numActions; ++j)
      {
        SamplerPtr reward = bernoulliSampler(randomGenerator->sampleDouble());
        SparseDoubleVectorPtr transitions = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
        
        std::vector<size_t> order;
        randomGenerator->sampleOrder(numStates, order);
        double Z = 0.0;
        for (size_t k = 0; k < numSuccessorsPerState; ++k)
        {
          double p = randomGenerator->sampleDouble();
          transitions->setElement(order[k], p);
          Z += p;
        }
        transitions->multiplyByScalar(1.0 / Z);
        res->setInfo(i, j, reward, transitions);
      }
    return res;
  }
  
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_SANDBOX_H_

