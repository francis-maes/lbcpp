/*-----------------------------------------.---------------------------------.
| Filename: FormulaLearnAndSearch.h        | Formula Learn&Search Algorithm  |
| Author  : Francis Maes                   |                                 |
| Started : 25/09/2011 21:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_
# define LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_

# include "PathsFormulaFeatureGenerator.h"
# include "NestedMonteCarloOptimizer.h"
# include "BeamSearchOptimizer.h"

namespace lbcpp
{

class FormulaRegressor : public Object
{
public:
  FormulaRegressor() 
  {
    featureGenerator = new PathsFormulaFeatureGenerator();
    featureGenerator->initialize(defaultExecutionContext(), gpExpressionClass);
    EnumerationPtr features = DoubleVector::getElementsEnumeration(featureGenerator->getOutputType());
    parameters = new DenseDoubleVector(features, doubleType);
  }

  void addExample(ExecutionContext& context, const GPExpressionPtr& formula, const std::vector<int>& formulaKey, DoubleVectorPtr& features, double trueScore)
  {
    getFeaturesIfNecessary(context, formula, formulaKey, features);

    double activation = features->dotProduct(parameters, 0);
    double score = 1.0 / (1.0 + exp(-features->dotProduct(parameters, 0)));
    errorStats.push(fabs(score - trueScore));
    double derivative = (score - trueScore) * score * (1 - score);
    features->addWeightedTo(parameters, 0, -derivative);
  }

  double predict(ExecutionContext& context, const GPExpressionPtr& formula, const std::vector<int>& formulaKey, DoubleVectorPtr& features, bool isFinalizedFormula = true) const
  {
    PathsFormulaFeatureGeneratorPtr fg = featureGenerator.staticCast<PathsFormulaFeatureGenerator>();
    fg->setDictionaryReadOnly(true);
    fg->setIsFinalizedFormula(isFinalizedFormula);
    getFeaturesIfNecessary(context, formula, formulaKey, features);
    double res = 1.0 / (1.0 + exp(-features->dotProduct(parameters, 0)));
    fg->setDictionaryReadOnly(false);
    fg->setIsFinalizedFormula(true);
    return res;
  }

  void flushInformation(ExecutionContext& context)
  {
    context.resultCallback("parameters l0norm", parameters->l0norm());
    context.resultCallback("parameters l1norm", parameters->l1norm());
    context.resultCallback("parameters l2norm", parameters->l2norm());
    //context.resultCallback("parameters", parameters->cloneAndCast<DenseDoubleVector>());
    context.resultCallback("absolute error", errorStats.getMean());
    errorStats.clear();

    // display most important parameters
    std::multimap<double, size_t> parametersByScore;
    size_t n = parameters->getNumValues();
    for (size_t i = 0; i < n; ++i)
      parametersByScore.insert(std::make_pair(-fabs(parameters->getValue(i)), i));
    context.informationCallback(T("Most important parameters:"));
    size_t i = 0;
    for (std::multimap<double, size_t>::iterator it = parametersByScore.begin(); it != parametersByScore.end() && i < 10; ++it, ++i)
    {
      size_t index = it->second;
      context.informationCallback(parameters->getElementName(index) + T(" [") + String(parameters->getValue(index)) + T("]"));
    }
  }

protected:
  FunctionPtr featureGenerator;
  DenseDoubleVectorPtr parameters;
  ScalarVariableStatistics errorStats;

  void getFeaturesIfNecessary(ExecutionContext& context, const GPExpressionPtr& formula, const std::vector<int>& formulaKey, DoubleVectorPtr& features) const
  {
    if (!features)
      features = featureGenerator->compute(context, formula).getObjectAndCast<DoubleVector>();
  }
};

typedef ReferenceCountedObjectPtr<FormulaRegressor> FormulaRegressorPtr;

class SuperFormulaPool : public ExecutionContextCallback
{
public:
  SuperFormulaPool(ExecutionContext& context, FormulaSearchProblemPtr problem, size_t maxFormulaSize = 6, size_t numInputSamples = 100)
    : problem(problem), regressor(new FormulaRegressor()), maxFormulaSize(maxFormulaSize), numInvalidFormulas(0), numEvaluations(0), creationFrequency(0), threadId(0)
  {
    FunctionPtr objective = problem->getObjective();
    objective->initialize(context, gpExpressionClass);

    problem->sampleInputs(context, numInputSamples, inputSamples);
  }
 
  bool doFormulaExists(GPExpressionPtr formula) const
    {checkCurrentThreadId(); return formulas.find(formula->toString()) != formulas.end();}

  typedef std::vector<int> FormulaKey;

  size_t getOrUpdateFormulaClass(ExecutionContext& context, const GPExpressionPtr& formula, const FormulaKey& key, bool verbose = false)
  {
    size_t res;
    KeyToFormulaClassMap::iterator it = keyToFormulaClassMap.find(key);
    if (it == keyToFormulaClassMap.end())
    {
      return (size_t)-1;
      //std::cout << "Formula: " << formula->toShortString() << " => new class " << info.formulaClass << std::endl;
    }
    else
    {
      // reuse and update existing formula class
      res = it->second;
      FormulaClassInfo& formulaClassInfo = formulaClasses[it->second];
      //std::cout << "Formula: " << formula->toShortString() << " => existing class " << it->second << " (" << formulaClassInfo.expression->toShortString() << ")" << std::endl;
      if (formula->size() < formulaClassInfo.expression->size())
      {
        if (verbose)
          context.informationCallback(T("Update formula class: ") + formulaClassInfo.expression->toShortString() + T(" => ") + formula->toShortString());
        formulaClassInfo.expression = formula; // keep the smallest formula
      }
    }
    return res;
  }

  bool addFormula(ExecutionContext& context, GPExpressionPtr formula, bool verbose = false)
  {
    checkCurrentThreadId();

    String str = formula->toString();
    FormulaInfoMap::iterator it = formulas.find(str);
    if (it != formulas.end())
      return it->second.isValidFormula();

    FormulaKey key;
    bool isValidFormula = problem->makeFormulaKey(formula, inputSamples, key);
    FormulaInfo info(formula, key);

    //std::cout << "Formula " << formula->toShortString() << " key = ";
    //for (size_t i = 0; i < key.size(); ++i) std::cout << key[i] << " ";
    //std::cout << std::endl;

    if (isValidFormula)
    {
      info.formulaClass = getOrUpdateFormulaClass(context, formula, key, verbose);
      if (info.formulaClass == (size_t)-1)
      {
        // create new formula class
        if (verbose)
          context.informationCallback(T("New formula class: ") + formula->toShortString());
        info.formulaClass = keyToFormulaClassMap[key] = formulaClasses.size();
        formulaClasses.push_back(FormulaClassInfo(formula, key));
        if (formulaClasses.size() % 1000 == 0)
          context.informationCallback(String((int)formulaClasses.size()) + T(" formula classes, last formula: ") + formula->toShortString());

        // add in bandit pool
        sortedFormulaClasses.insert(std::make_pair(-DBL_MAX, info.formulaClass));
      }
    }
    else
    {
      //std::cout << "Invalid formula: " << formula->toShortString() << std::endl;
      key.clear();
      ++numInvalidFormulas;
    }

    formulas[str] = info;
    return info.isValidFormula();
  }


  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    checkCurrentThreadId();

    FunctionWorkUnitPtr wu = workUnit.staticCast<FunctionWorkUnit>();
    GPExpressionPtr formula = wu->getInputs()[0].getObjectAndCast<GPExpression>();
    double reward = result.toDouble();
    receiveReward(*this->context, formula, reward);
  }

  void receiveReward(ExecutionContext& context, GPExpressionPtr formula, double reward)
  {
    checkCurrentThreadId();
//    std::cout << formula->toShortString() << " ==> " << reward << std::endl;

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
    checkCurrentThreadId();

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

  struct SurrogateObjective : public SimpleUnaryFunction
  {
    SurrogateObjective(SuperFormulaPool* pthis, FunctionPtr formulaObjective)
      : SimpleUnaryFunction(decisionProblemStateClass, doubleType), pthis(pthis), formulaObjective(formulaObjective) {}

    SuperFormulaPool* pthis;
    FunctionPtr formulaObjective;

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      pthis->checkCurrentThreadId();

      const RPNGPExpressionBuilderStatePtr& state = input.getObjectAndCast<RPNGPExpressionBuilderState>();

      if (state->isFinalState())
      {
        GPExpressionPtr expression = state->getExpression();
        if (pthis->doFormulaExists(expression))
          return DBL_MAX; // already known formula

        FormulaKey key;
        bool isValidFormula = pthis->problem->makeFormulaKey(expression, pthis->inputSamples, key);
        if (!isValidFormula)
          return DBL_MAX; // invalid formula

        size_t formulaClass = pthis->getOrUpdateFormulaClass(context, expression, key);
        if (formulaClass != (size_t)-1)
          return DBL_MAX; // already known formula equivalence class

        DoubleVectorPtr features;
        double score = pthis->regressor->predict(context, expression, std::vector<int>(), features);
        //double reward = formulaObjective->compute(context, expression).toDouble();
        return 1.0 - score; // transform into score to minimize
      }
      else
      {
        const std::vector<GPExpressionPtr>& stack = state->getStack();
        double score = 0.0;
        for (size_t i = 0; i < stack.size(); ++i)
        {
          DoubleVectorPtr features;
          // todo: there may be a cache here !
          score += pthis->regressor->predict(context, stack[i], std::vector<int>(), features, false);
        }
        return 1.0 - score; // transform into score to minimize
      }
    }
  };

  void createNewFormula(ExecutionContext& context)
  {
    checkCurrentThreadId();

    OptimizationProblemPtr surrogateProblem = new OptimizationProblem(new SurrogateObjective(this, problem->getObjective()));
    DecisionProblemStatePtr initialState = problem->makeGPBuilderState(10); // FIXME: max formula size

    static const bool useNestedMonteCarlo = false;

    if (useNestedMonteCarlo)
    {
      // Nested Monte Carlo
      OptimizerPtr nestedMC = new NestedMonteCarloOptimizer(initialState, 2, 1); // level 1, one iteration
      OptimizerStatePtr state = nestedMC->optimize(context, surrogateProblem);
      GPExpressionBuilderStatePtr bestFinalState = state->getBestSolution().getObjectAndCast<GPExpressionBuilderState>();
      if (bestFinalState)
      {
        GPExpressionPtr formula = bestFinalState->getExpression();
        //context.informationCallback(T("New formula: ") + formula->toShortString() + T(" ==> ") + String(state->getBestScore()));
        addFormula(context, formula, true);
      }
      else
        context.informationCallback(T("Failed to create new formula"));
    }
    else
    {
      static const int beamSize = 1000;
      static const int numBests = 100;

      // Beam Search
      OptimizerPtr beamSearch = new BeamSearchOptimizer(initialState, beamSize);
      context.enterScope(T("Beam Search"));
      BeamSearchOptimizerStatePtr state = beamSearch->optimize(context, surrogateProblem).staticCast<BeamSearchOptimizerState>();
      context.leaveScope(state->getBestScore());
      const BeamSearchOptimizerState::SortedStateMap& finalStates = state->getFinalStates();
      size_t i = 0;
      for (BeamSearchOptimizerState::SortedStateMap::const_iterator it = finalStates.begin(); it != finalStates.end() && i < numBests; ++it, ++i)
      {
        if (it->first == DBL_MAX)
          break;
        GPExpressionPtr formula = it->second.staticCast<GPExpressionBuilderState>()->getExpression();
        //context.informationCallback(T("New formula ") + String((int)i+1) + T(": ") + formula->toShortString() + T(" ==> ") + String(it->first));
        addFormula(context, formula, true);
      }
    }
  }

  void play(ExecutionContext& context, size_t iterationNumber, size_t numTimeSteps, size_t creationFrequency)
  {
    checkCurrentThreadId();

    this->creationFrequency = creationFrequency;
    this->context = &context;

    context.enterScope(T("Iteration ") + String((int)iterationNumber));
    context.resultCallback(T("iteration"), iterationNumber);

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
    regressor->flushInformation(context);
    context.leaveScope();
  }

  void displayBestFormulas(ExecutionContext& context)
  {
    checkCurrentThreadId();

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

    FormulaClassInfo& bestFormula = formulaClasses[formulasByMeanReward.rbegin()->second];
    context.resultCallback(T("bestFormulaScore"), bestFormula.statistics.getMean());
    context.resultCallback(T("bestFormulaPlayCount"), bestFormula.statistics.getCount());
    context.resultCallback(T("bestFormulaSize"), bestFormula.expression->size());
    context.resultCallback(T("banditPoolSize"), formulasByMeanReward.size());
  }

  size_t getNumFormulas() const
    {return formulas.size();}

  size_t getNumInvalidFormulas() const
    {return numInvalidFormulas;}

  size_t getNumFormulaClasses() const
    {return formulaClasses.size();}

  const GPExpressionPtr& getFormulaClassExpression(size_t index) const
    {jassert(index < formulaClasses.size()); return formulaClasses[index].expression;}

protected:
  FormulaSearchProblemPtr problem;
  FormulaRegressorPtr regressor;
  size_t maxFormulaSize;
  size_t creationFrequency;
  ExecutionContextPtr context;

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
  size_t numEvaluations;

  void receiveReward(ExecutionContext& context, size_t index, double reward)
  {
    checkCurrentThreadId();
    ++numEvaluations;

    // update stats
    FormulaClassInfo& formula = formulaClasses[index];
    formula.statistics.push(reward);

    // update bandit score (reinsert into bandit pool)
    double newScore = formula.statistics.getMean() + 2.0 / formula.statistics.getCount();
    sortedFormulaClasses.insert(std::make_pair(-newScore, index));

    // add regression example, and eventually create new formula
    regressor->addExample(context, formula.expression, formula.key, formula.features, reward);
    if (creationFrequency && (numEvaluations % creationFrequency == 0))
      createNewFormula(context);
  }

  Thread::ThreadID threadId;

  void checkCurrentThreadId() const
  {
    if (threadId)
    {
      jassert(Thread::getCurrentThreadId() == threadId);
    }
    else
      const_cast<SuperFormulaPool* >(this)->threadId = Thread::getCurrentThreadId();
  }
};

class FormulaLearnAndSearch : public WorkUnit
{
public:
  FormulaLearnAndSearch() : formulaInitialSize(4), numInitialIterations(1), creationFrequency(10), numIterations(10), iterationsLength(1000) {}

  virtual Variable run(ExecutionContext& context)
  {   
    SuperFormulaPool pool(context, problem, formulaInitialSize + 1);

/*    
    FunctionPtr featureGenerator = new PathsFormulaFeatureGenerator();
    featureGenerator->initialize(context, gpExpressionClass);
    GPExpressionPtr testExpr = GPExpression::createFromString(context, "B(add, V(rk), U(inverse,V(tk)))", problem->getVariables());
    context.resultCallback(T("Test Features"), featureGenerator->compute(context, testExpr));
    return true;*/

    size_t iteration = 0;
    if (formulaInitialSize > 0)
    {
      if (!generateInitialFormulas(context, pool))
        return false;

      // initial plays
      size_t n = pool.getNumFormulaClasses();
      for (size_t i = 0; i < numInitialIterations; ++i)
        pool.play(context, iteration++, iterationsLength, 0);
      context.informationCallback(T("End of initial plays"));
    }

    // all the rest
    for (size_t i = 0; i < numIterations; ++i)
      pool.play(context, iteration++, iterationsLength, creationFrequency);

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
    
    size_t n = pool.getNumFormulaClasses();
    context.enterScope(String((int)n) + T(" valid formula equivalence classes"));
    if (n < 200)
      for (size_t i = 0; i < n; ++i)
        context.informationCallback(pool.getFormulaClassExpression(i)->toShortString());
    context.leaveScope();

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
