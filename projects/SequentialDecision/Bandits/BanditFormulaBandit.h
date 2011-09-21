/*-----------------------------------------.---------------------------------.
| Filename: BanditFormulaBandit.h          | Bandit Formula Bandit           |
| Author  : Francis Maes                   |                                 |
| Started : 21/09/2011 12:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_BANDIT_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_BANDIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "DiscreteBanditExperiment.h"
# include "../GP/GPExpressionBuilder.h"
# include <algorithm>

namespace lbcpp
{

class EvaluateBanditFormulaWorkUnit : public WorkUnit
{
public:
   EvaluateBanditFormulaWorkUnit(size_t minArms, size_t maxArms, double maxExpectedReward, size_t horizon, DiscreteBanditPolicyPtr policy)
    : minArms(minArms), maxArms(maxArms), maxExpectedReward(maxExpectedReward), horizon(horizon), policy(policy)  {}

  virtual String toShortString() const
    {return T("Evaluate ") + policy->toShortString();}

  virtual Variable run(ExecutionContext& context)
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
    
    policy->initialize(arms.size());

    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }
    return horizon * bestRewardExpectation - sumOfRewards; // regret
  }
 
protected:
  size_t minArms;
  size_t maxArms;
  double maxExpectedReward;
  size_t horizon;
  DiscreteBanditPolicyPtr policy;

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }
};

class FormulaPool
{
public:
  FormulaPool(size_t minArms, size_t maxArms, double maxExpectedReward, size_t horizon)
    : minArms(minArms), maxArms(maxArms), maxExpectedReward(maxExpectedReward), horizon(horizon) {}
    
  void addFormula(const GPExpressionPtr& expression)
  {
    formulas.insert(std::make_pair(-DBL_MAX, Formula(expression)));
  }

  void playBestFormula(ExecutionContext& context)
  {
    std::multimap<double, Formula>::iterator it = formulas.begin();
    Formula formula = it->second;
    formulas.erase(it);
    
    WorkUnitPtr workUnit = new EvaluateBanditFormulaWorkUnit(minArms, maxArms, maxExpectedReward, horizon, gpExpressionDiscreteBanditPolicy(formula.expression));
    double score = context.run(workUnit, false).toDouble();
    double reward = exp(-score / 100.0);
    
    formula.statistics.push(reward);
    double newScore = formula.statistics.getMean() + 2.0 / formula.statistics.getCount();
    formulas.insert(std::make_pair(-newScore, formula));
  }
  
  void displayBestFormulas(ExecutionContext& context)
  {
    size_t n = 10;
    size_t i = 1;
    
    for (std::multimap<double, Formula>::const_iterator it = formulas.begin(); i < n && it != formulas.end(); ++it, ++i)
    {
      context.informationCallback(T("[") + String((int)i) + T("] ") + it->second.expression->toShortString() + T(" meanReward = ") + String(it->second.statistics.getMean())
       + T(" playedCount = ") + String(it->second.statistics.getCount()));
    }
  }

protected:
  size_t minArms;
  size_t maxArms;
  double maxExpectedReward;
  size_t horizon;

  struct Formula
  {
    Formula(const GPExpressionPtr& expression)
      : expression(expression) {}
  
    GPExpressionPtr expression;
    ScalarVariableStatistics statistics;
  };
  
  std::multimap<double, Formula> formulas;
  //std::vector<Formula> currentlyEvaluatedFormulas;
};

class BanditFormulaBanditWorkUnit : public WorkUnit
{
public:
  BanditFormulaBanditWorkUnit() : maxSize(3), orderBasedKey(false), minArms(2), maxArms(10), maxExpectedReward(1.0), horizon(1000) {}

  typedef std::vector<int> FormulaKey;
  
  virtual Variable run(ExecutionContext& context)
  {
    std::vector< std::vector<double> > inputSamples;
    makeInputSamples(context, orderBasedKey ? 1000 : 100, inputSamples);
    
    RPNGPExpressionBuilderStatePtr state = new RPNGPExpressionBuilderState("Coucou", gpExpressionDiscreteBanditPolicyVariablesEnumeration, FunctionPtr(), maxSize);
    std::map<FormulaKey, GPExpressionPtr> formulas;
    enumerateAllFormulas(context, state, inputSamples, formulas);
    
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");
    
    FormulaPool pool(minArms, maxArms, maxExpectedReward, horizon);
    for (std::map<FormulaKey, GPExpressionPtr>::const_iterator it = formulas.begin(); it != formulas.end(); ++it)
      pool.addFormula(it->second);
    
    for (size_t i = 0; i < 100; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      for (size_t j = 0; j < formulas.size(); ++j)
        pool.playBestFormula(context);
      pool.displayBestFormulas(context);
      context.leaveScope();
    }
    return true;
  }
  
  void makeInputSamples(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    for (size_t i = 0; i < count - 2; ++i)
    {
      std::vector<double> input(7);
      input[0] = juce::jmax(1, (int)pow(10.0, random->sampleDouble(0, 5))); // t
      input[1] = random->sampleDouble(0.0, 1.0); // rk1
      input[2] = random->sampleDouble(0.0, 0.5); // sk1
      input[3] = juce::jmax(1, (int)(input[0] * random->sampleDouble(0.0, 1.0))); // tk1
      input[4] = random->sampleDouble(0.0, 1.0); // rk2
      input[5] = random->sampleDouble(0.0, 0.5); // sk2
      input[6] = juce::jmax(1, (int)(input[0] * random->sampleDouble(0.0, 1.0))); // tk2
      res[i] = input;
    }

    std::vector<double> smallest(7);
    smallest[0] = 1.0;
    smallest[1] = smallest[2] = smallest[4] = smallest[5] = 0.0;
    smallest[3] = smallest[6] = 1.0;
    res[count - 2] = smallest;

    std::vector<double> highest(7);
    highest[0] = 100000;
    highest[1] = highest[4] = 1.0;
    highest[2] = highest[5] = 0.5;
    highest[3] = highest[6] = 100000;
    res[count - 1] = highest;
  }
  
  struct CompareSecond
  {
    bool operator() (const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
    {
      if (a.second == b.second)
        return a.first < b.first;
      else
        return a.second < b.second;
    }
  };
  
  bool makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples, FormulaKey& res)
  {
    res.resize(inputSamples.size());

    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      std::vector<double> v(4);
      v[0] = inputSamples[i][1];
      v[1] = inputSamples[i][2];
      v[2] = inputSamples[i][3];
      v[3] = inputSamples[i][0];
      double value1 = expression->compute(&v[0]);
      if (!isNumberValid(value1))
        return false;

      v[0] = inputSamples[i][4];
      v[1] = inputSamples[i][5];
      v[2] = inputSamples[i][6];
      v[3] = inputSamples[i][0];
      double value2 = expression->compute(&v[0]);
      if (!isNumberValid(value2))
        return false;

      if (orderBasedKey)
      {
        if (value1 < value2)
          res[i] = 0;
        else if (value1 == value2)
          res[i] = 1;
        else if (value1 > value2)
          res[i] = 2;
      }
      else
      {
        res[i] = (int)(value1 * 10000) + (int)(value2 * 1000000);
      }
    }
    return true;
  }
  
  static String formulaKeyToString(const FormulaKey& key)
  {
    String res;
    for (size_t i = 0; i < key.size(); ++i)
      res += String(key[i]) + T(";");
    return res;
  }
  
  void enumerateAllFormulas(ExecutionContext& context, RPNGPExpressionBuilderStatePtr state, const std::vector< std::vector<double> >& inputSamples, std::map<FormulaKey, GPExpressionPtr>& res)
  {
    if (state->isFinalState())
    {
      GPExpressionPtr formula = state->getExpression();
      jassert(formula);
      
      FormulaKey formulaKey;
      if (!makeFormulaKey(formula, inputSamples, formulaKey))
      {
        //context.informationCallback("Invalid formula: " + formula->toShortString());
        return; // invalid formula
      }
      //context.informationCallback("Formula: " + formula->toShortString() + " --> " + formulaKeyToString(formulaKey));
        
      std::map<FormulaKey, GPExpressionPtr>::iterator it = res.find(formulaKey);
      if (it == res.end())
      {
        //context.informationCallback("Formula: " + formula->toShortString());
        res[formulaKey] = formula;
        if (res.size() % 1000 == 0)
          context.informationCallback(String((int)res.size()) + T(" formulas, last formula: ") + formula->toShortString());
      }
      else if (formula->size() < it->second->size())
        it->second = formula; // keep the smallest formula
    }
    else
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable stateBackup;
        Variable action = actions->getElement(i);
        double reward;
        state->performTransition(context, action, reward, &stateBackup);
        enumerateAllFormulas(context, state, inputSamples, res);
        state->undoTransition(context, stateBackup);
      }
    }
  }
  
protected:
  friend class BanditFormulaBanditWorkUnitClass;

  size_t maxSize;
  bool orderBasedKey;
  size_t minArms;
  size_t maxArms;
  double maxExpectedReward;
  size_t horizon;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_BANDIT_H_
