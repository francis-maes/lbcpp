/*-----------------------------------------.---------------------------------.
| Filename: FormulaLearnAndSearch.h        | Formula Learn&Search Algorithm  |
| Author  : Francis Maes                   |                                 |
| Started : 25/09/2011 21:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_
# define LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_

# include "BanditFormulaOptimizer.h"
#ifdef JUCE_WIN32
# include <hash_map>
# include <hash_set>

# define std_hash_map stdext::hash_map
# define std_hash_set stdext::hash_set

#else
# define STDEXT_NAMESPACE __gnu_cxx
# include <ext/hash_map>
# include <ext/hash_set>
# define std_hash_map STDEXT_NAMESPACE::hash_map
# define std_hash_set STDEXT_NAMESPACE::hash_set
#endif // JUCE_WIN32

namespace lbcpp
{

class SuperFormulaPool : public ExecutionContextCallback
{
public:
  SuperFormulaPool(ExecutionContext& context, FormulaSearchProblemPtr problem, size_t numInputSamples = 100)
    : problem(problem)
  {
    problem->sampleInputs(context, numInputSamples, inputSamples);
  }

  bool addFormula(ExecutionContext& context, GPExpressionPtr formula)
  {
    String str = formula->toString();
    FormulaInfoMap::iterator it = formulas.find(str);
    if (it != formulas.end())
      return it->second.isValidFormula();

    FormulaKey key;
    bool isValidFormula = problem->makeFormulaKey(formula, inputSamples, key);
    FormulaInfo info(formula, key);

    if (isValidFormula)
    {
      KeyToFormulaClassMap::iterator it = keyToFormulaClassMap.find(key);
      if (it == keyToFormulaClassMap.end())
      {
        // create new formula class
        info.formulaClass = formulaClasses.size();
        formulaClasses.push_back(FormulaClassInfo(formula, key));
        if (formulaClasses.size() % 1000 == 0)
          context.informationCallback(String((int)formulaClasses.size()) + T(" formula classes, last formula: ") + formula->toShortString());
      }
      else
      {
        // reuse and update existing formula class
        info.formulaClass = it->second;
        FormulaClassInfo& formulaClassInfo = formulaClasses[it->second];
        if (formula->size() < formulaClassInfo.expression->size())
          formulaClassInfo.expression = formula; // keep the smallest formula
      }
    }
    else
      key.clear();

    formulas[str] = info;
    return info.isValidFormula();
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
    std::map<GPExpressionPtr, size_t>::iterator it = currentlyEvaluatedFormulas.find(formula);
    jassert(it != currentlyEvaluatedFormulas.end());
    size_t index = it->second;
    currentlyEvaluatedFormulas.erase(it);
    receiveReward(index, reward);
  }

  size_t getNumCurrentlyEvaluatedFormulas() const
    {return currentlyEvaluatedFormulas.size();}

  void playBestFormula(ExecutionContext& context)
  {
    if (sortedFormulaClasses.empty())
      return;

    std::multimap<double, size_t>::iterator it = sortedFormulaClasses.begin();
    size_t index = it->second;
    sortedFormulaClasses.erase(it);

    FormulaClassInfo& formula = formulaClasses[index];
    
    WorkUnitPtr workUnit = functionWorkUnit(problem->getObjective(), std::vector<Variable>(1, formula.expression));
    if (context.isMultiThread())
    {
      jassert(currentlyEvaluatedFormulas.find(formula.expression) == currentlyEvaluatedFormulas.end());
      currentlyEvaluatedFormulas[formula.expression] = index;
      context.pushWorkUnit(workUnit, this, false);
    }
    else
    {
      double reward = context.run(workUnit, false).toDouble();
      receiveReward(index, reward);
    }
  }

  void play(size_t numTimeSteps, size_t creationFrequency)
  {
    // TODO
  }

  size_t getNumFormulaClasses() const
    {return formulaClasses.size();}

protected:
  typedef std::vector<int> FormulaKey;

  FormulaSearchProblemPtr problem;
  std::vector< std::vector<double> > inputSamples;

  struct FormulaInfo
  {
    FormulaInfo(const GPExpressionPtr& expression, const FormulaKey& key)
      : expression(expression), key(key), formulaClass((size_t)-1) {}
    FormulaInfo() : formulaClass((size_t)-1) {}

    GPExpressionPtr expression;
    FormulaKey key;
    size_t formulaClass;

    bool isValidFormula() const
      {return key.size() > 0;}
  };

  typedef std::map<String, FormulaInfo> FormulaInfoMap;
  FormulaInfoMap formulas;

  struct FormulaClassInfo
  {
    FormulaClassInfo(GPExpressionPtr expression, const FormulaKey& key)
      : expression(expression), key(key) {}
    FormulaClassInfo() {}

    GPExpressionPtr expression;
    FormulaKey key;
    ScalarVariableStatistics statistics;
    DoubleVectorPtr features;
  };
  std::vector<FormulaClassInfo> formulaClasses;

  typedef std::map<FormulaKey, size_t> KeyToFormulaClassMap;
  KeyToFormulaClassMap keyToFormulaClassMap;

  std::multimap<double, size_t> sortedFormulaClasses;
  std::map<GPExpressionPtr, size_t> currentlyEvaluatedFormulas;

  void receiveReward(size_t index, double reward)
  {
    FormulaClassInfo& formula = formulaClasses[index];
    formula.statistics.push(reward);
    double newScore = formula.statistics.getMean() + 2.0 / formula.statistics.getCount();
    sortedFormulaClasses.insert(std::make_pair(-newScore, index));
  }
};

class FormulaLearnAndSearch : public WorkUnit
{
public:
  FormulaLearnAndSearch() : formulaInitialSize(4), creationFrequency(10), numIterations(10), iterationsLength(1000) {}

  virtual Variable run(ExecutionContext& context)
  {   
    SuperFormulaPool pool(context, problem);

    if (!generateInitialFormulas(context, pool))
      return false;

    // initial plays
    size_t n = pool.getNumFormulaClasses();
    pool.play(5 * n, 0);

    // other plays
    for (size_t i = 0; i < numIterations; ++i)
      pool.play(iterationsLength, creationFrequency);

    return Variable();
  }

protected:
  friend class FormulaLearnAndSearchClass;

  FormulaSearchProblemPtr problem;
  size_t formulaInitialSize;
  size_t creationFrequency;
  size_t numIterations;
  size_t iterationsLength;

  bool generateInitialFormulas(ExecutionContext& context, SuperFormulaPool& pool)
    {enumerateAllFormulas(context, problem->makeGPBuilderState(formulaInitialSize), pool); return true;}
  
  void enumerateAllFormulas(ExecutionContext& context, GPExpressionBuilderStatePtr state, SuperFormulaPool& pool)
  {
    if (state->isFinalState())
      pool.addFormula(context, state->getExpression());
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
        enumerateAllFormulas(context, state, pool);
        state->undoTransition(context, stateBackup);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_
