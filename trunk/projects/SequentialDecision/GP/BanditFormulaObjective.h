/*-----------------------------------------.---------------------------------.
| Filename: EvaluateBanditFormulaObjective.h| Evaluate Bandit Formula        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_
# define LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_

# include "FormulaSearchProblem.h"
# include "../Bandits/DiscreteBanditExperiment.h"

namespace lbcpp
{

class BanditFormulaObjective : public SimpleUnaryFunction
{
public:
  BanditFormulaObjective(size_t minArms = 2, size_t maxArms = 10, double maxExpectedReward = 1.0, size_t horizon = 100)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), minArms(minArms), maxArms(maxArms), maxExpectedReward(maxExpectedReward), horizon(horizon)  {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& expression) const
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

typedef ReferenceCountedObjectPtr<BanditFormulaObjective> BanditFormulaObjectivePtr;

class BanditFormulaSearchProblem : public FormulaSearchProblem
{
public:
  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return gpExpressionDiscreteBanditPolicyVariablesEnumeration;}

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpLog; i <= gpInverse; ++i)
      unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpMax; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    for (size_t i = 0; i < count - 2; ++i)
    {
      std::vector<double> input(8);
      input[0] = random->sampleDouble(0.0, 1.0); // rk1
      input[1] = random->sampleDouble(0.0, 0.5); // sk1
      input[2] = juce::jmax(1, (int)(input[0] * random->sampleDouble(0.0, 1.0))); // tk1
      input[3] = juce::jmax(1, (int)pow(10.0, random->sampleDouble(0, 5))); // t
      input[4] = random->sampleDouble(0.0, 1.0); // rk2
      input[5] = random->sampleDouble(0.0, 0.5); // sk2
      input[6] = juce::jmax(1, (int)(input[0] * random->sampleDouble(0.0, 1.0))); // tk2
      input[7] = input[3];
      res[i] = input;
    }

    std::vector<double> smallest(8);
    smallest[0] = smallest[1] = smallest[4] = smallest[5] = 0.0;
    smallest[2] = smallest[3] = smallest[6] = smallest[7] = 1.0;
    res[count - 2] = smallest;

    std::vector<double> highest(8);
    highest[0] = highest[4] = 1.0;
    highest[1] = highest[5] = 0.5;
    highest[2] = highest[3] = highest[6] = highest[7] = 100000;
    res[count - 1] = highest;
  }

  virtual bool makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples, std::vector<int>& res) const
  {
    res.resize(inputSamples.size());
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      const std::vector<double>& variables = inputSamples[i];
      double value1 = expression->compute(&variables[0]);
      if (!isNumberValid(value1))
        return false;
      double value2 = expression->compute(&variables[4]);
      if (!isNumberValid(value2))
        return false;

      if (value1 < value2)
        res[i] = 0;
      else if (value1 == value2)
        res[i] = 1;
      else if (value1 > value2)
        res[i] = 2;
    }
    return true;
  }

protected:
  friend class BanditFormulaSearchProblemClass;

  BanditFormulaObjectivePtr objective;
};


}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_
