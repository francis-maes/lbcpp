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
  BanditFormulaSearchProblem() : numArmsInSampling(5) {}

  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return gpExpressionDiscreteBanditPolicyVariablesEnumeration;}

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpOpposite; i <= gpAbs; ++i)
      unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpPow; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    size_t index = 0;

    for (; index < count; ++index)
    {
      double t = juce::jmax(1, (int)pow(10.0, random->sampleDouble(0, 5)));
      jassert(numArmsInSampling >= 2);
      std::vector<double> input(4 * numArmsInSampling);
      size_t i = 0;
      input[i++] = 0.0;
      input[i++] = 0.0;
      input[i++] = 1.0;
      input[i++] = t;
      for (size_t j = 0; j < numArmsInSampling - 1; ++j)
      {
        input[i++] = random->sampleDouble(0.0, 1.0); // rk1
        input[i++] = random->sampleDouble(0.0, 0.5); // sk1
        input[i++] = juce::jmax(1, (int)(t * random->sampleDouble(0.0, 1.0))); // tk1
        input[i++] = t;
      }
      res[index] = input;
    }
  }

  struct DoubleThresholdedComparator
  {
    bool operator()(double a, double b)
    {
      //static const double epsilon = 1e-16;
      return a < b;// && fabs(a - b) > epsilon;
    }
  };

  virtual FormulaKeyPtr makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    typedef std::multimap<double, size_t, DoubleThresholdedComparator> SortedValuesMap;

    FormulaKeyPtr res = new FormulaKey(inputSamples.size() * numArmsInSampling);
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      const std::vector<double>& variables = inputSamples[i];
      
      SortedValuesMap values;
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        double value = expression->compute(&variables[j * 4]);
        if (!isNumberValid(value))
          return FormulaKeyPtr();
        values.insert(std::make_pair(value, j));
      }
      jassert(values.size() == numArmsInSampling);
  
      jassert(numArmsInSampling < 128);
      double prevValue = DBL_MAX;
      for (SortedValuesMap::const_iterator it = values.begin(); it != values.end(); ++it)
      {
        double value = it->first;
        bool isHigherThanPrevious = (it != values.begin() || value > prevValue);
        prevValue = value;
        res->pushByte((unsigned char)(it->second + (isHigherThanPrevious ? 128 : 0)));
      }
    }
    return res;
  }

protected:
  friend class BanditFormulaSearchProblemClass;

  BanditFormulaObjectivePtr objective;
  size_t numArmsInSampling;
};


}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATE_BANDIT_FORMULA_OBJECTIVE_H_
