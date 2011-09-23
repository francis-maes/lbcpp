/*-----------------------------------------.---------------------------------.
| Filename: BanditFormulaOptimizer.h       | Bandit Formula Optimizer        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 19:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_OPTIMIZER_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_OPTIMIZER_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "BanditFormulaObjective.h"
# include "LearningRuleFormulaObjective.h"
# include "GPExpressionBuilder.h"
# include <algorithm>

namespace lbcpp
{

class FormulaPool : public ExecutionContextCallback
{
public:
  FormulaPool(FunctionPtr objective)
    : objective(objective) {}
    
  void addFormula(const GPExpressionPtr& expression)
  {
    formulas.insert(std::make_pair(-DBL_MAX, Formula(expression)));
  }

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    FunctionWorkUnitPtr wu = workUnit.staticCast<FunctionWorkUnit>();
    GPExpressionPtr formula = wu->getInputs()[0].getObjectAndCast<GPExpression>();
    double reward = result.toDouble();
    receiveReward(formula, reward);
  }

  void receiveReward(GPExpressionPtr formula, double reward)
  {
    std::map<GPExpressionPtr, Formula>::iterator it = currentlyEvaluatedFormulas.find(formula);
    jassert(it != currentlyEvaluatedFormulas.end());
    Formula f = it->second;
    currentlyEvaluatedFormulas.erase(it);
    receiveReward(f, reward);
  }

  size_t getNumCurrentlyEvaluatedFormulas() const
    {return currentlyEvaluatedFormulas.size();}

  void playBestFormula(ExecutionContext& context)
  {
    if (formulas.empty())
      return;

    std::multimap<double, Formula>::iterator it = formulas.begin();
    Formula formula = it->second;
    formulas.erase(it);
    
    WorkUnitPtr workUnit = functionWorkUnit(objective, std::vector<Variable>(1, formula.expression));
    if (context.isMultiThread())
    {
      jassert(currentlyEvaluatedFormulas.find(formula.expression) == currentlyEvaluatedFormulas.end());
      currentlyEvaluatedFormulas[formula.expression] = formula;
      context.pushWorkUnit(workUnit, this, false);
    }
    else
    {
      double reward = context.run(workUnit, false).toDouble();
      receiveReward(formula, reward);
    }
  }
  
  void displayBestFormulas(ExecutionContext& context)
  {
    std::multimap<double, Formula> formulasByMeanReward;
    for (std::multimap<double, Formula>::const_iterator it = formulas.begin(); it != formulas.end(); ++it)
      formulasByMeanReward.insert(std::make_pair(it->second.statistics.getMean(), it->second));

    size_t n = 10;
    size_t i = 1;
    for (std::multimap<double, Formula>::const_reverse_iterator it = formulasByMeanReward.rbegin(); i < n && it != formulasByMeanReward.rend(); ++it, ++i)
    {
      context.informationCallback(T("[") + String((int)i) + T("] ") + it->second.expression->toShortString() + T(" meanReward = ") + String(it->second.statistics.getMean())
       + T(" playedCount = ") + String(it->second.statistics.getCount()));
    }
  }

  void run(ExecutionContext& context, const std::vector<GPExpressionPtr>& formulas, size_t numIterations)
  {
    for (size_t i = 0; i < formulas.size(); ++i)
      addFormula(formulas[i]);
    
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));

      if (context.isMultiThread())
      {
        for (size_t j = 0; j < formulas.size(); ++j)
        {
          context.progressCallback(new ProgressionState(j+1,formulas.size(), T("Turns")));
          playBestFormula(context);
          context.flushCallbacks();
          //context.informationCallback(T("Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
          
          while (getNumCurrentlyEvaluatedFormulas() >= 10)
          {
            //context.informationCallback(T("Waiting - Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
            Thread::sleep(10);
            context.flushCallbacks();
          }
        }
        while (getNumCurrentlyEvaluatedFormulas() > 0)
        {
          Thread::sleep(10);
          context.flushCallbacks();
        }
/*
        size_t count = 0;
        while (count < formulas.size())
        {
          while (getNumCurrentlyEvaluatedFormulas() < 20 && count < formulas.size())
          {
            ++count;
            playBestFormula(context);
            context.flushCallbacks();
          }
          Thread::sleep(1);
        }*/
      }
      else
      {
        for (size_t j = 0; j < formulas.size(); ++j)
          playBestFormula(context);
      }

      displayBestFormulas(context);
      context.leaveScope();
    }
  }

  std::vector<GPExpressionPtr> getBestFormulas(size_t count) const
  {
    std::multimap<double, Formula> formulasByMeanReward;
    for (std::multimap<double, Formula>::const_iterator it = formulas.begin(); it != formulas.end(); ++it)
      formulasByMeanReward.insert(std::make_pair(it->second.statistics.getMean(), it->second));
    std::vector<GPExpressionPtr> res;
    for (std::multimap<double, Formula>::const_reverse_iterator it = formulasByMeanReward.rbegin(); res.size() < count && it != formulasByMeanReward.rend(); ++it)
      res.push_back(it->second.expression);
    return res;
  }

protected:
  FunctionPtr objective;
  struct Formula
  {
    Formula(const GPExpressionPtr& expression = GPExpressionPtr())
      : expression(expression) {}
  
    GPExpressionPtr expression;
    ScalarVariableStatistics statistics;
  };
  std::multimap<double, Formula> formulas;
  std::map<GPExpressionPtr, Formula> currentlyEvaluatedFormulas;

  void receiveReward(Formula formula, double reward)
  {
    formula.statistics.push(reward);
    double newScore = formula.statistics.getMean() + 2.0 / formula.statistics.getCount();
    formulas.insert(std::make_pair(-newScore, formula));
  }
};


class BanditFormulaWorkUnit : public WorkUnit
{
public:
  BanditFormulaWorkUnit() : objective(new BanditFormulaObjective(2, 10, 1.0, 1000)), numIterations(10) {}

  virtual Variable run(ExecutionContext& context)
  {   
    if (!objective->initialize(context, gpExpressionClass))
      return false;

    std::vector<GPExpressionPtr> formulas;
    if (!loadFormulasFromFile(context, formulasFile, formulas))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");
 
    GPExpressionPtr baselineExpression;
    if (baseline.isNotEmpty())
    {
      EnumerationPtr variablesEnumeration = learningRuleFormulaVariablesEnumeration; // FIXME
      baselineExpression = GPExpression::createFromString(context, baseline, variablesEnumeration);

      ScalarVariableStatisticsPtr baselineStats = new ScalarVariableStatistics("baseline");
      for (size_t i = 0; i < 100; ++i)
        baselineStats->push(objective->compute(context, baselineExpression).toDouble());
      context.informationCallback(T("Baseline: ") + baselineStats->toShortString());
      context.resultCallback(T("baseline"), baselineStats);
    }

    FormulaPool pool(objective);
    if (baselineExpression)
      pool.addFormula(baselineExpression);
    pool.run(context, formulas, numIterations);

    std::vector<GPExpressionPtr> bestFormulas = pool.getBestFormulas(10);
    for (size_t i = 0; i < bestFormulas.size(); ++i)
    {
      GPExpressionPtr formula = bestFormulas[i];
      context.enterScope(T("Test ") + formula->toShortString());
      double testScore = objective.staticCast<LearningRuleFormulaObjective>()->testFormula(context, formula);
      context.leaveScope(testScore);
    }

    return true;
  }
  
protected:
  friend class BanditFormulaWorkUnitClass;

  File formulasFile;
  FunctionPtr objective;
  String baseline;
  size_t numIterations;

  bool loadFormulasFromFile(ExecutionContext& context, const File& formulasFile, std::vector<GPExpressionPtr>& res) const
  {
    //EnumerationPtr variablesEnumeration = gpExpressionDiscreteBanditPolicyVariablesEnumeration;
    EnumerationPtr variablesEnumeration = learningRuleFormulaVariablesEnumeration; // FIXME

    InputStream* istr = formulasFile.createInputStream();
    if (!istr)
    {
      context.errorCallback(T("Could not open file ") + formulasFile.getFullPathName());
      return false;
    }
    while (!istr->isExhausted())
    {
      String line = istr->readNextLine();
      if (!line.isEmpty())
      {
        GPExpressionPtr expression = GPExpression::createFromString(context, line, variablesEnumeration);
        if (expression)
          res.push_back(expression);
      }
    }
    delete istr;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_BANDIT_H_
