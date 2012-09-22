/*-----------------------------------------.---------------------------------.
| Filename: BanditNoParamSearchProblem.h   | Search policies without any param|
| Author  : Francis Maes                   |                                 |
| Started : 17/10/2011 10:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_SEARCH_PROBLEM_BANDIT_NO_PARAM_H_
# define LBCPP_GP_SEARCH_PROBLEM_BANDIT_NO_PARAM_H_

# include "BanditFormulaSearchProblem.h"
# include "../Bandits/FindBanditsFormula.h"

namespace lbcpp
{

class BanditNoParamObjective : public SimpleUnaryFunction
{
public:
  BanditNoParamObjective() : SimpleUnaryFunction(gpExpressionClass, regretScoreObjectClass) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();
    
    RandomGeneratorPtr random = context.getRandomGenerator();
    
    double numArmsRandom = random->sampleDouble(1.0, 7.0);
    size_t numArms = (size_t)pow(2.0, numArmsRandom);
    size_t horizon = (size_t)pow(10.0, random->sampleDouble(2.0, 5.0));
    
    DiscreteBanditPolicyPtr policy = new Formula5TunedDiscreteBanditPolicy(formula, horizon);
    
    std::vector<SamplerPtr> arms(numArms);
    std::vector<double> expectedRewards(numArms);
    double bestRewardExpectation = 0.0;
    
    for (size_t i = 0; i < numArms; ++i)
    {
      double p = random->sampleDouble();
      arms[i] = bernoulliSampler(p);
      expectedRewards[i] = p;
      if (p > bestRewardExpectation)
        bestRewardExpectation = p;
    }
    
    double regret = sampleRegret(context, policy, horizon, arms, expectedRewards, bestRewardExpectation);
    return new RegretScoreObject(regret / horizon, 1.0);
  }
 
protected:
  friend class BanditNoParamObjectiveClass;

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }

  double sampleRegret(ExecutionContext& context, DiscreteBanditPolicyPtr pol, size_t horizon, const std::vector<SamplerPtr>& arms, const std::vector<double>& expectedRewards, double bestRewardExpectation) const
  {
    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);    
    DiscreteBanditPolicyPtr policy = pol->cloneAndCast<DiscreteBanditPolicy>();
    policy->initialize(arms.size());
    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }

  /*  if (isSimpleRegret)
    {
      double bestReward = 0.0;
      size_t bestEstimatedArm = 0;
      for (size_t i = 0; i < arms.size(); ++i)
      {
        double reward = policy->getRewardEmpiricalMean(i);
        if (reward > bestReward)
          bestReward = reward, bestEstimatedArm = i;
      }
      return bestRewardExpectation - expectedRewards[bestEstimatedArm]; // simple regret 
    }
    else*/
      return horizon * bestRewardExpectation - sumOfRewards; // regret
  }
};

typedef ReferenceCountedObjectPtr<BanditNoParamObjective> BanditNoParamObjectivePtr;

extern EnumerationPtr banditNoParamVariablesEnumeration;

class BanditNoParamSearchProblem : public FormulaSearchProblem
{
public:
  BanditNoParamSearchProblem() : objective(new BanditNoParamObjective()) {}
  
  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return banditNoParamVariablesEnumeration;}

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpOpposite; i <= gpAbs; ++i)
      if (i != gpExp)
        unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpMax; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    size_t index = 0;

    for (; index < count; ++index)
    {
      std::vector<double> input(3);

      double numArmsRandom = random->sampleDouble(1.0, 14.0);
      input[0] = (size_t)pow(2.0, numArmsRandom);
      input[1] = (size_t)(input[0] * pow(2.0, random->sampleDouble(1.0, 20.0 - numArmsRandom)));
      input[2] = juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1));

      res[index] = input;
    }
  }

  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    BinaryKeyPtr res = new BinaryKey(inputSamples.size() * sizeof (juce::int64));
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      double value = expression->compute(&inputSamples[i][0]);
      if (!isNumberValid(value))
        return BinaryKeyPtr();
      res->pushInteger((juce::int64)(value * 100000));
    }
    return res;
  }

protected:
  friend class BanditNoParamSearchProblemClass;

  BanditNoParamObjectivePtr objective;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GP_SEARCH_PROBLEM_BANDIT_NO_PARAM_H_
