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
# include "../WorkUnit/GPSandBox.h"
# include "../Bandits/FindBanditsFormula.h"

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
         + T(" playedCount = ") + String((int)playedCount));
    }
  }

  void run(ExecutionContext& context, const std::vector<GPExpressionPtr>& formulas, size_t numIterations, size_t iterationsLength, size_t bestFormulaIndex = (size_t)-1, std::vector<double>* bestPlayedPercents = NULL)
  {
    this->formulas = formulas;
    policy->initialize(formulas.size());
    totalReward = 0.0;

    size_t bestPlayedCount = 0;
    if (bestPlayedPercents)
      bestPlayedPercents->resize(numIterations);

    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));

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
        double bestPlayedPercent = bestPlayedCount / (double)((i+1)*iterationsLength);
        if (bestPlayedPercents)
          (*bestPlayedPercents)[i] = bestPlayedPercent;
        context.resultCallback(T("bestPlayedPercent"), 100.0 * bestPlayedPercent);
        //context.resultCallback(T("bestPlayedCount"), policy->getBanditPlayedCount(bestFormulaIndex));
      }
      //context.resultCallback(T("totalReward"), totalReward);

      displayBestFormulas(context);
      context.leaveScope();
      context.progressCallback(new ProgressionState(i+1, numIterations, T("Iterations")));
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

  struct Run : public WorkUnit
  {
    Run(juce::uint32 seed, const std::vector<GPExpressionPtr>& formulas, const DiscreteBanditPolicyPtr& policy, const FunctionPtr& objective, size_t numIterations, size_t iterationsLength, size_t bestFormulaIndex)
      : seed(seed), formulas(formulas), policy(policy), objective(objective), numIterations(numIterations), iterationsLength(iterationsLength), bestFormulaIndex(bestFormulaIndex) {}

    virtual Variable run(ExecutionContext& context)
    {
      context.getRandomGenerator()->setSeed(seed);
      FormulaPool pool(policy->cloneAndCast<DiscreteBanditPolicy>(), objective);
      pool.run(context, formulas, numIterations, iterationsLength, bestFormulaIndex, &bestPlayedPercents);
      hasFoundBestBandit = (pool.getBestFormulas(1)[0] == formulas[bestFormulaIndex]);
      return true;
    }

    virtual String toShortString() const
      {return T("Run with seed ") + String((int)seed);}

    juce::uint32 seed;
    const std::vector<GPExpressionPtr>& formulas;
    DiscreteBanditPolicyPtr policy;
    FunctionPtr objective;
    size_t numIterations;
    size_t iterationsLength;
    size_t bestFormulaIndex;

    std::vector<double> bestPlayedPercents;
    bool hasFoundBestBandit;
  };


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

    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(ucb1DiscreteBanditPolicy(2.0));
    policies.push_back(ucb1TunedDiscreteBanditPolicy());
    policies.push_back(klucbDiscreteBanditPolicy(0.0));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(1.0));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(1.5));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(2.0));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(5.0));

    for (size_t i = 0; i < policies.size(); ++i)
    {
      DiscreteBanditPolicyPtr policy = policies[i];
      context.enterScope(T("Policy ") + policy->toShortString());

      size_t numRuns = 10;
      CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Making ") + String((int)numRuns) + T(" runs"), numRuns));
      for (size_t i = 0; i < numRuns; ++i)
        workUnit->setWorkUnit(i, new Run((juce::uint32)(1664 + 51 * i), formulas, policy, objective, numIterations, iterationsLength, bestFormulaIndex));
      workUnit->setPushChildrenIntoStackFlag(true);
      context.run(workUnit);
      
      std::vector<ScalarVariableStatistics> bestPlayedPercentsStats(numIterations);
      size_t numSuccess = 0;
      for (size_t i = 0; i < numRuns; ++i)
      {
        std::vector<double>& bestPlayedPercents = workUnit->getWorkUnit(i).staticCast<Run>()->bestPlayedPercents;
        jassert(bestPlayedPercents.size() == numIterations);
        for (size_t j = 0; j < numIterations; ++j)
          bestPlayedPercentsStats[j].push(bestPlayedPercents[j]);
        if (workUnit->getWorkUnit(i).staticCast<Run>()->hasFoundBestBandit)
          ++numSuccess;
      }
      context.informationCallback(T("Success rate: ") + String((int)numSuccess) + T(" / ") + String((int)numRuns));

      context.enterScope(T("Results"));
      for (size_t i = 0; i < numIterations; ++i)
      {
        context.enterScope(T("Iteration ") + String((int)i));
        context.resultCallback(T("iteration"), i);
        context.resultCallback(T("score"), bestPlayedPercentsStats[i].getMean());
        context.resultCallback(T("score stddev"), bestPlayedPercentsStats[i].getStandardDeviation());
        context.leaveScope();
      }
      context.leaveScope();

      context.leaveScope();
    }
    return true;
  }
  
protected:
  friend class BanditFormulaWorkUnitClass;

  FormulaSearchProblemPtr problem;
  File formulasFile;
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

    typedef std::multimap<double, std::pair<GPExpressionPtr, ScalarVariableStatisticsPtr> > SortedFormulasMap;

    SortedFormulasMap sortedFormulas;
    for (size_t i = 0; i < formulas.size(); ++i)
    {
      GPExpressionPtr formula = formulas[i];
      ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics();
      for (size_t j = 0; j < numEstimations; ++j)
        stats->push(objective->compute(context, formula).toDouble());
      double score = stats->getMean();
      sortedFormulas.insert(std::make_pair(isMinimizationProblem ? score : -score, std::make_pair(formula, stats)));
      context.progressCallback(new ProgressionState(i, formulas.size(), T("Formulas")));
    }
    context.leaveScope();

    size_t i = 1;
    context.enterScope(T("Best formulas"));
    for (SortedFormulasMap::const_iterator it = sortedFormulas.begin(); it != sortedFormulas.end(); ++it)
    {
      ScalarVariableStatisticsPtr stats = it->second.second;

      context.enterScope(T("Formula ") + String((int)i) + T(": ") + it->second.first->toShortString());
      context.resultCallback(T("rank"), i);
      context.resultCallback(T("mean"), stats->getMean());
      context.resultCallback(T("stddev"), stats->getStandardDeviation());
      context.resultCallback(T("min"), stats->getMinimum());
      context.resultCallback(T("max"), stats->getMaximum());
      context.resultCallback(T("formula"), it->second.first);
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
