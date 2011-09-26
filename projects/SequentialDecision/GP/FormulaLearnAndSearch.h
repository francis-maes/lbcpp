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
# include "NestedMonteCarloOptimizer.h"

namespace lbcpp
{

class FormulaRegressor : public Object
{
public:
  FormulaRegressor() 
  {
    parameters = new DenseDoubleVector(); // todo: features enumeration
  }

  void addExample(ExecutionContext& context, const GPExpressionPtr& formula, const std::vector<int>& formulaKey, DoubleVectorPtr& features, double reward)
  {
    getFeaturesIfNecessary(context, formula, formulaKey, features);

    double score = features->dotProduct(parameters, 0);
    // todo: gradient of the loss / sigmoid

  }

  double predict(ExecutionContext& context, const GPExpressionPtr& formula, const std::vector<int>& formulaKey, DoubleVectorPtr& features) const
  {
    getFeaturesIfNecessary(context, formula, formulaKey, features);
    return 1.0 / (1.0 + exp(-features->dotProduct(parameters, 0)));
  }

protected:
  DenseDoubleVectorPtr parameters;

  void getFeaturesIfNecessary(ExecutionContext& context, const GPExpressionPtr& formula, const std::vector<int>& formulaKey, DoubleVectorPtr& features) const
  {
    if (!features)
    {
      // TODO
      //features = ...;
    }
  }
};

typedef ReferenceCountedObjectPtr<FormulaRegressor> FormulaRegressorPtr;


class SuperFormulaPool : public ExecutionContextCallback
{
public:
  SuperFormulaPool(ExecutionContext& context, FormulaSearchProblemPtr problem, size_t maxFormulaSize = 12, size_t numInputSamples = 100)
    : problem(problem), regressor(new FormulaRegressor()), maxFormulaSize(maxFormulaSize), numInvalidFormulas(0)
  {
    problem->sampleInputs(context, numInputSamples, inputSamples);
  }
 
  bool doFormulaExists(GPExpressionPtr formula) const
    {return formulas.find(formula->toString()) != formulas.end();}

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
        info.formulaClass = keyToFormulaClassMap[key] = formulaClasses.size();
        formulaClasses.push_back(FormulaClassInfo(formula, key));
        if (formulaClasses.size() % 1000 == 0)
          context.informationCallback(String((int)formulaClasses.size()) + T(" formula classes, last formula: ") + formula->toShortString());

        // add in bandit pool
        sortedFormulaClasses.insert(std::make_pair(-DBL_MAX, info.formulaClass));
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
    {
      key.clear();
      ++numInvalidFormulas;
    }

    formulas[str] = info;
    return info.isValidFormula();
  }


  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    FunctionWorkUnitPtr wu = workUnit.staticCast<FunctionWorkUnit>();
    GPExpressionPtr formula = wu->getInputs()[0].getObjectAndCast<GPExpression>();
    double reward = result.toDouble();
    receiveReward(defaultExecutionContext(), formula, reward);
  }

  void receiveReward(ExecutionContext& context, GPExpressionPtr formula, double reward)
  {
    std::map<GPExpressionPtr, size_t>::iterator it = currentlyEvaluatedFormulas.find(formula);
    jassert(it != currentlyEvaluatedFormulas.end());
    size_t index = it->second;
    currentlyEvaluatedFormulas.erase(it);
    receiveReward(context, index, reward);
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
      receiveReward(context, index, reward);
    }
  }

  struct DecoratedObjective : public SimpleUnaryFunction
  {
    DecoratedObjective(SuperFormulaPool* pthis, FunctionPtr formulaObjective)
      : SimpleUnaryFunction(decisionProblemStateClass, doubleType), pthis(pthis), formulaObjective(formulaObjective) {}

    SuperFormulaPool* pthis;
    FunctionPtr formulaObjective;

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      const GPExpressionBuilderStatePtr& state = input.getObjectAndCast<GPExpressionBuilderState>();
      GPExpressionPtr expression = state->getExpression();
      if (pthis->doFormulaExists(expression))
        return DBL_MAX;
      double reward = formulaObjective->compute(context, expression).toDouble();
      return 1.0 - reward; // transform into score to minimize
    }
  };

  void createNewFormula(ExecutionContext& context)
  {
    OptimizationProblemPtr nestedMCProblem = new OptimizationProblem(new DecoratedObjective(this, problem->getObjective()));
    DecisionProblemStatePtr initialState = problem->makeGPBuilderState(maxFormulaSize);
    OptimizerPtr nestedMC = new NestedMonteCarloOptimizer(initialState, 1, 1); // level 1, one iteration
    context.enterScope(T("Nested MC"));
    OptimizerStatePtr state = nestedMC->compute(context, nestedMCProblem).getObjectAndCast<OptimizerState>();
    context.leaveScope(state->getBestScore());
    GPExpressionBuilderStatePtr bestFinalState = state->getBestSolution().getObjectAndCast<GPExpressionBuilderState>();
    GPExpressionPtr formula = bestFinalState->getExpression();
    context.informationCallback(T("New formula: ") + formula->toShortString());
    addFormula(context, formula);
  }

  void play(ExecutionContext& context, size_t numTimeSteps, size_t creationFrequency)
  {
    context.enterScope(T("Play ") + String((int)numTimeSteps) + T(" with creation frequency = ") + String((int)creationFrequency));

    if (context.isMultiThread())
    {
      for (size_t i = 0; i < numTimeSteps; ++i)
      {
        context.progressCallback(new ProgressionState(i + 1, numTimeSteps, T("Steps")));
        playBestFormula(context);
        context.flushCallbacks();
        //context.informationCallback(T("Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
        
        while (getNumCurrentlyEvaluatedFormulas() >= 10)
        {
          //context.informationCallback(T("Waiting - Num played: ") + String((int)j + 1) + T(" num currently evaluated: " ) + String((int)getNumCurrentlyEvaluatedFormulas()));
          Thread::sleep(10);
          context.flushCallbacks();
        }

        if (creationFrequency && (i % creationFrequency) == 0)
          createNewFormula(context);
      }
      while (getNumCurrentlyEvaluatedFormulas() > 0)
      {
        Thread::sleep(10);
        context.flushCallbacks();
      }
    }
    else
    {
      for (size_t j = 0; j < formulas.size(); ++j)
        playBestFormula(context);
    }

    displayBestFormulas(context);
    context.leaveScope();
  }

  void displayBestFormulas(ExecutionContext& context)
  {
    std::multimap<double, size_t> formulasByMeanReward;
    for (std::multimap<double, size_t>::const_iterator it = sortedFormulaClasses.begin(); it != sortedFormulaClasses.end(); ++it)
      formulasByMeanReward.insert(std::make_pair(formulaClasses[it->second].statistics.getMean(), it->second));

    size_t n = 10;
    size_t i = 1;
    for (std::multimap<double, size_t>::const_reverse_iterator it = formulasByMeanReward.rbegin(); i < n && it != formulasByMeanReward.rend(); ++it, ++i)
    {
      FormulaClassInfo& formula = formulaClasses[it->second];
      context.informationCallback(T("[") + String((int)i) + T("] ") + formula.expression->toShortString() + T(" meanReward = ") + String(formula.statistics.getMean())
        + T(" playedCount = ") + String(formula.statistics.getCount()));
    }
  }

  size_t getNumFormulas() const
    {return formulas.size();}

  size_t getNumInvalidFormulas() const
    {return numInvalidFormulas;}

  size_t getNumFormulaClasses() const
    {return formulaClasses.size();}

protected:
  typedef std::vector<int> FormulaKey;

  FormulaSearchProblemPtr problem;
  FormulaRegressorPtr regressor;
  size_t maxFormulaSize;

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

    bool isAssociatedToClass() const
      {return formulaClass != (size_t)-1;}
  };

  typedef std::map<String, FormulaInfo> FormulaInfoMap;
  FormulaInfoMap formulas;
  size_t numInvalidFormulas;

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

  void receiveReward(ExecutionContext& context, size_t index, double reward)
  {
    // update stats
    FormulaClassInfo& formula = formulaClasses[index];
    formula.statistics.push(reward);

    // add regression example
    // regressor->addExample(context, formula.expression, formula.key, formula.features, reward);

    // update bandit score (reinsert into bandit pool)
    double newScore = formula.statistics.getMean() + 2.0 / formula.statistics.getCount();
    sortedFormulaClasses.insert(std::make_pair(-newScore, index));
  }
};

class FormulaLearnAndSearch : public WorkUnit
{
public:
  FormulaLearnAndSearch() : formulaInitialSize(4), numInitialIterations(1), creationFrequency(10), numIterations(10), iterationsLength(1000) {}

  virtual Variable run(ExecutionContext& context)
  {   
    SuperFormulaPool pool(context, problem);

    if (formulaInitialSize > 0)
    {
      if (!generateInitialFormulas(context, pool))
        return false;

      // initial plays
      size_t n = pool.getNumFormulaClasses();
      for (size_t i = 0; i < numInitialIterations; ++i)
        pool.play(context, iterationsLength, 0);
      context.informationCallback(T("End of initial plays"));
    }

    // all the rest
    for (size_t i = 0; i < numIterations; ++i)
      pool.play(context, iterationsLength, creationFrequency);

    return Variable();
  }

protected:
  friend class FormulaLearnAndSearchClass;

  FormulaSearchProblemPtr problem;
  size_t formulaInitialSize;
  size_t numInitialIterations;
  size_t creationFrequency;
  size_t numIterations;
  size_t iterationsLength;

  bool generateInitialFormulas(ExecutionContext& context, SuperFormulaPool& pool)
  {
    context.enterScope(T("Generating initial formulas up to size ") + String((int)formulaInitialSize));
    size_t numFinalStates = 0;
    enumerateAllFormulas(context, problem->makeGPBuilderState(formulaInitialSize), pool, numFinalStates);

    context.informationCallback(String((int)numFinalStates) + T(" final states"));
    context.informationCallback(String((int)pool.getNumFormulas()) + T(" formulas"));
    context.informationCallback(String((int)pool.getNumInvalidFormulas()) + T(" invalid formulas"));
    context.informationCallback(String((int)pool.getNumFormulaClasses()) + T(" valid formula equivalence classes"));
    context.leaveScope(pool.getNumFormulaClasses());
    return true;
  }
  
  void enumerateAllFormulas(ExecutionContext& context, GPExpressionBuilderStatePtr state, SuperFormulaPool& pool, size_t& numFinalStates)
  {
    if (state->isFinalState())
    {
      ++numFinalStates;
      pool.addFormula(context, state->getExpression());
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
        enumerateAllFormulas(context, state, pool, numFinalStates);
        state->undoTransition(context, stateBackup);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_
