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
# include "../Bandits/DiscreteBanditPolicy.h"
# include <algorithm>

namespace lbcpp
{

class RegretScoreObject : public ScoreObject
{
public:
  RegretScoreObject(double regret = 1.0, double referenceRegret = 1.0)
    : regret(regret), reward(juce::jmax(0.0, (referenceRegret - regret) / referenceRegret)) {}

  virtual double getScoreToMinimize() const
    {return regret;}

  double getRegret() const
    {return regret;}

  double getReward() const
    {return reward;}

protected:
  friend class RegretScoreObjectClass;

  double regret;
  double reward;
};

typedef ReferenceCountedObjectPtr<RegretScoreObject> RegretScoreObjectPtr;
extern ClassPtr regretScoreObjectClass;

class BanditFormulaObjective : public SimpleUnaryFunction
{
public:
  BanditFormulaObjective(bool isSimpleRegret = false, size_t worstNumSamples = 1, size_t minArms = 2, size_t maxArms = 10, double maxExpectedReward = 1.0, size_t horizon = 100)
    : SimpleUnaryFunction(gpExpressionClass, regretScoreObjectClass), isSimpleRegret(isSimpleRegret), worstNumSamples(worstNumSamples), minArms(minArms), maxArms(maxArms), maxExpectedReward(maxExpectedReward), horizon(horizon)
    {jassert(maxArms >= minArms);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& expression) const
  {
    GPExpressionPtr formula = expression.getObjectAndCast<GPExpression>();
    
    double bestRewardExpectation, meanRewardExpectation;
    double regret = sampleWorstRegret(context, formula, bestRewardExpectation, meanRewardExpectation);
    double meanUniformRegret = bestRewardExpectation - meanRewardExpectation;
    return new RegretScoreObject(regret, isSimpleRegret ? 1 : horizon);
  }
 
protected:
  friend class BanditFormulaObjectiveClass;

  bool isSimpleRegret;
  size_t worstNumSamples;

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

  double sampleRegret(ExecutionContext& context, GPExpressionPtr formula, const std::vector<SamplerPtr>& arms, const std::vector<double>& expectedRewards, double bestRewardExpectation) const
  {
    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);    
    DiscreteBanditPolicyPtr policy = gpExpressionDiscreteBanditPolicy(formula);
    policy->initialize(arms.size());
    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }

    if (isSimpleRegret)
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
    else
      return horizon * bestRewardExpectation - sumOfRewards; // regret
  }
 
  double sampleWorstRegret(ExecutionContext& context, GPExpressionPtr formula, double& bestRewardExpectation, double& meanRewardExpectation) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    bestRewardExpectation = 0.0;
    meanRewardExpectation = 0.0;

    std::vector<SamplerPtr> arms(random->sampleSize(minArms, maxArms));
    std::vector<double> expectedRewards(arms.size());
    
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double p = random->sampleDouble(0.0, maxExpectedReward);
      if (p > bestRewardExpectation) 
        bestRewardExpectation = p;
      meanRewardExpectation += p;
      arms[i] = bernoulliSampler(p);
      expectedRewards[i] = p;
    }
    meanRewardExpectation /= arms.size();

    double worstRegret = -DBL_MAX;
    for (size_t i = 0; i < worstNumSamples; ++i)
    {
      double regret = sampleRegret(context, formula, arms, expectedRewards, bestRewardExpectation);
      if (regret > worstRegret)
        worstRegret = regret;
    }
    return worstRegret;
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
      double t = juce::jmax(1, (int)pow(10.0, random->sampleDouble(-0.1, 5)));
      jassert(numArmsInSampling >= 2);
      std::vector<double> input(4 * numArmsInSampling);
      size_t i = 0;
      /*input[i++] = 0.0;
      input[i++] = 0.0;
      input[i++] = 1.0;
      input[i++] = t;
      */
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        input[i++] = juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1)); // rk1
        input[i++] = juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1)); // sk1
        input[i++] = juce::jlimit(1, (int)t, (int)(t * random->sampleDouble(-0.1, 1.1))); // tk1
        input[i++] = t;
      }
      res[index] = input;
    }
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
      {return (left.second != right.second ? left.second < right.second : left.first < right.first);}
  };

  virtual FormulaKeyPtr makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    FormulaKeyPtr res = new FormulaKey(inputSamples.size() * numArmsInSampling);
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      const std::vector<double>& variables = inputSamples[i];
      
      std::vector< std::pair<size_t, double> > values(numArmsInSampling);
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        double value = expression->compute(&variables[j * 4]);
        if (!isNumberValid(value))
          return FormulaKeyPtr();
        values[j] = std::make_pair(j, value);
      }
      std::sort(values.begin(), values.end(), ValueComparator());
  
      jassert(numArmsInSampling <= 128);
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        bool isHigherThanPrevious = (j > 0 && values[j].second > values[j-1].second);
        res->pushByte((unsigned char)(values[j].first + (isHigherThanPrevious ? 128 : 0)));
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
