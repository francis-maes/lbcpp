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
# include "LearningRuleFormulaObjective.h"
# include <algorithm>

namespace lbcpp
{

class FormulaPool : public ExecutionContextCallback
{
public:
  FormulaPool(DiscreteBanditPolicyPtr policy, FunctionPtr objective)
    : policy(policy), objective(objective) {}
    
  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    FunctionWorkUnitPtr wu = workUnit.staticCast<FunctionWorkUnit>();
    GPExpressionPtr formula = wu->getInputs()[0].getObjectAndCast<GPExpression>();
    double reward = result.toDouble();
    receiveReward(formula, reward);
  }

  void receiveReward(GPExpressionPtr formula, double reward)
  {
    std::map<GPExpressionPtr, size_t>::iterator it = currentlyEvaluatedFormulas.find(formula);
    jassert(it != currentlyEvaluatedFormulas.end());
    size_t index = it->second;
    currentlyEvaluatedFormulas.erase(it);
    receiveReward(index, reward);
  }

  size_t getNumCurrentlyEvaluatedFormulas() const
    {return currentlyEvaluatedFormulas.size();}

  size_t playBestFormula(ExecutionContext& context)
  {
    if (formulas.empty())
      return (size_t)-1;

    size_t index = policy->selectNextBandit(context);

    GPExpressionPtr formula = formulas[index];
    WorkUnitPtr workUnit = functionWorkUnit(objective, std::vector<Variable>(1, formula));
    if (false)//context.isMultiThread())
    {
      jassert(currentlyEvaluatedFormulas.find(formula) == currentlyEvaluatedFormulas.end());
      currentlyEvaluatedFormulas[formula] = index;
      context.pushWorkUnit(workUnit, this, false);
    }
    else
    {
      double reward = context.run(workUnit, false).toDouble();
      receiveReward(index, reward);
    }

    return index;
  }
  
  void displayBestFormulas(ExecutionContext& context)
  {
    std::multimap<double, size_t> formulasByMeanReward;
    for (size_t i = 0; i < formulas.size(); ++i)
      formulasByMeanReward.insert(std::make_pair(policy->getRewardEmpiricalMean(i), i));

    size_t n = 10;
    size_t i = 1;
    for (std::multimap<double, size_t>::reverse_iterator it = formulasByMeanReward.rbegin(); i < n && it != formulasByMeanReward.rend(); ++it, ++i)
    {
      GPExpressionPtr expression = formulas[it->second];
      size_t playedCount = policy->getBanditPlayedCount(it->second);
      context.informationCallback(T("[") + String((int)i) + T("] ") + expression->toShortString() + T(" meanReward = ") + String(it->first)
         + T(" playedCount = ") + String(playedCount));
    }
  }

  void run(ExecutionContext& context, const std::vector<GPExpressionPtr>& formulas, size_t numIterations, size_t iterationsLength, size_t bestFormulaIndex = (size_t)-1)
  {
    this->formulas = formulas;
    policy->initialize(formulas.size());
    totalReward = 0.0;
    
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      size_t bestPlayedCount = 0;

      if (false)//context.isMultiThread())
      {
        for (size_t j = 0; j < iterationsLength; ++j)
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
      }
      else
      {
        for (size_t j = 0; j < iterationsLength; ++j)
        {
          size_t index = playBestFormula(context);
          if (index == bestFormulaIndex)
            ++bestPlayedCount;
        }
      }

      context.resultCallback(T("iteration"), i);
      if (bestFormulaIndex != (size_t)-1)
      {
        context.resultCallback(T("bestPlayedCountPercent"), 100.0 * bestPlayedCount / (double)iterationsLength);
        context.resultCallback(T("bestPlayedCount"), policy->getBanditPlayedCount(bestFormulaIndex));
      }
      context.resultCallback(T("totalReward"), totalReward);

      displayBestFormulas(context);
      context.leaveScope();
    }
  }

  std::vector<GPExpressionPtr> getBestFormulas(size_t count)
  {
    std::multimap<double, GPExpressionPtr> formulasByMeanReward;
    for (size_t i = 0; i < formulas.size(); ++i)
      formulasByMeanReward.insert(std::make_pair(policy->getRewardEmpiricalMean(i), formulas[i]));
    std::vector<GPExpressionPtr> res;
    for (std::multimap<double, GPExpressionPtr>::reverse_iterator it = formulasByMeanReward.rbegin(); res.size() < count && it != formulasByMeanReward.rend(); ++it)
      res.push_back(it->second);
    return res;
  }

protected:
  DiscreteBanditPolicyPtr policy;
  FunctionPtr objective;
  std::vector<GPExpressionPtr> formulas;
  double totalReward;

/*  struct Formula
  {
    Formula(const GPExpressionPtr& expression = GPExpressionPtr())
      : expression(expression) {}
  
    GPExpressionPtr expression;
    ScalarVariableStatistics statistics;
  };
  std::multimap<double, Formula> formulas;*/
  std::map<GPExpressionPtr, size_t> currentlyEvaluatedFormulas;

  void receiveReward(size_t index, double reward)
  {
    totalReward += reward;
    policy->updatePolicy(index, reward);
  }
};


class BanditFormulaWorkUnit : public WorkUnit
{
public:
  BanditFormulaWorkUnit() : numIterations(10), iterationsLength(0) {}

  virtual Variable run(ExecutionContext& context)
  {   
    FunctionPtr objective = problem->getObjective();

    if (!objective->initialize(context, gpExpressionClass))
      return false;

    std::vector<GPExpressionPtr> formulas;
    if (!GPExpression::loadFormulasFromFile(context, formulasFile, problem->getVariables(), formulas))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");

    size_t bestFormulaIndex = (size_t)-1;
    // tmp !
    for (size_t i = 0; i < formulas.size(); ++i)
      if (formulas[i]->toShortString() == T("(rk)+(inverse(tk))"))
      {
        bestFormulaIndex = i;
        break;
      }

    if (!iterationsLength)
      iterationsLength = formulas.size();

    FormulaPool pool(policy, objective);
    pool.run(context, formulas, numIterations, iterationsLength, bestFormulaIndex);
    return true;
  }
  
protected:
  friend class BanditFormulaWorkUnitClass;

  FormulaSearchProblemPtr problem;
  File formulasFile;
  DiscreteBanditPolicyPtr policy;
  size_t numIterations;
  size_t iterationsLength;
};

class PlayFormulasNTimes : public WorkUnit
{
public:
  PlayFormulasNTimes() : numEstimations(1000), isMinimizationProblem(true) {}

  virtual Variable run(ExecutionContext& context)
  {   
    FunctionPtr objective = problem->getObjective();
    if (!objective->initialize(context, gpExpressionClass))
      return false;

    std::vector<GPExpressionPtr> formulas;
    if (!GPExpression::loadFormulasFromFile(context, formulasFile, problem->getVariables(), formulas))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");

    context.enterScope(T("Evaluating formulas ") + String((int)numEstimations) + T(" times"));
    std::multimap<double, GPExpressionPtr> sortedFormulas;
    for (size_t i = 0; i < formulas.size(); ++i)
    {
      GPExpressionPtr formula = formulas[i];
      double score = 0.0;
      for (size_t j = 0; j < numEstimations; ++j)
        score += objective->compute(context, formula).toDouble();
      score /= numEstimations;
      sortedFormulas.insert(std::make_pair(isMinimizationProblem ? score : -score, formula));
      context.progressCallback(new ProgressionState(i, formulas.size(), T("Formulas")));
    }
    context.leaveScope();

    size_t i = 1;
    context.enterScope(T("Best formulas"));
    for (std::multimap<double, GPExpressionPtr>::const_iterator it = sortedFormulas.begin(); it != sortedFormulas.end(); ++it)
    {
      context.enterScope(T("Formula ") + String((int)i));
      context.resultCallback(T("rank"), i);
      context.resultCallback(T("meanScore"), isMinimizationProblem ? it->first : -it->first);
      context.resultCallback(T("formula"), it->second);
      context.leaveScope(it->first);
      ++i;
    }
    context.leaveScope();
    return true;
  }
  
protected:
  friend class PlayFormulasNTimesClass;

  FormulaSearchProblemPtr problem;
  File formulasFile;
  size_t numEstimations;
  bool isMinimizationProblem;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_BANDIT_H_
