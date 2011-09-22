/*-----------------------------------------.---------------------------------.
| Filename: EvaluateBanditFormulaObjective.h| Evaluate Bandit Formula        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_
# define LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_

# include "../Bandits/DiscreteBanditExperiment.h"

namespace lbcpp
{

class BanditFormulaObjective : public SimpleUnaryFunction
{
public:
  BanditFormulaObjective(size_t minArms = 2, size_t maxArms = 2, double maxExpectedReward = 1.0, size_t horizon = 100)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), minArms(minArms), maxArms(maxArms), maxExpectedReward(maxExpectedReward), horizon(horizon)  {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& expression)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    double bestRewardExpectation = 0.0;
    std::vector<SamplerPtr> arms(random->sampleSize(minArms, maxArms));
    std::vector<double> expectedRewards(arms.size());
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double p = random->sampleDouble(0.0, maxExpectedReward);
      if (p > bestRewardExpectation) 
        bestRewardExpectation = p;
      arms[i] = bernoulliSampler(p);
      expectedRewards[i] = p;
    }
    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);
    
    DiscreteBanditPolicyPtr policy = gpExpressionDiscreteBanditPolicy(expression.getObjectAndCast<GPExpression>());
    policy->initialize(arms.size());

    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }
    double regret = horizon * bestRewardExpectation - sumOfRewards; // regret
    double uniformPolicyRegret = 0.0;
    for (size_t i = 0; i < arms.size(); ++i)
      uniformPolicyRegret += (bestRewardExpectation - expectedRewards[i]) * horizon / (double)arms.size();
    return exp(-regret / uniformPolicyRegret);
  }
 
protected:
  friend class BanditFormulaObjectiveClass;

  size_t minArms;
  size_t maxArms;
  double maxExpectedReward;
  size_t horizon;

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_
