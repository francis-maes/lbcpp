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

class FormulaPool
{
public:
  FormulaPool(FunctionPtr objective)
    : objective(objective) {}
    
  void addFormula(const GPExpressionPtr& expression)
  {
    formulas.insert(std::make_pair(-DBL_MAX, Formula(expression)));
  }

  void playBestFormula(ExecutionContext& context)
  {
    std::multimap<double, Formula>::iterator it = formulas.begin();
    Formula formula = it->second;
    formulas.erase(it);
    
    WorkUnitPtr workUnit = functionWorkUnit(objective, std::vector<Variable>(1, formula.expression));
    double reward = context.run(workUnit, false).toDouble();
    
    formula.statistics.push(reward);
    double newScore = formula.statistics.getMean() + 2.0 / formula.statistics.getCount();
    formulas.insert(std::make_pair(-newScore, formula));
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
      for (size_t j = 0; j < formulas.size(); ++j)
        playBestFormula(context);
      displayBestFormulas(context);
      context.leaveScope();
    }
  }

protected:
  FunctionPtr objective;

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


class BanditFormulaWorkUnit : public WorkUnit
{
public:
  BanditFormulaWorkUnit() : objective(new BanditFormulaObjective(2, 10, 1.0, 1000)) {}

  virtual Variable run(ExecutionContext& context)
  {   
    if (!objective->initialize(context, gpExpressionClass))
      return false;

    std::vector<GPExpressionPtr> formulas;
    if (!loadFormulasFromFile(context, formulasFile, formulas))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");

    FormulaPool pool(objective);
    pool.run(context, formulas, 100);
    return true;
  }
  
protected:
  friend class BanditFormulaWorkUnitClass;

  File formulasFile;
  FunctionPtr objective;

  bool loadFormulasFromFile(ExecutionContext& context, const File& formulasFile, std::vector<GPExpressionPtr>& res) const
  {
    //EnumerationPtr variablesEnumeration = gpExpressionDiscreteBanditPolicyVariablesEnumeration;
    EnumerationPtr variablesEnumeration = learningRuleFormulaVariablesEnumeration;

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
