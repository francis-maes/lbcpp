/*-----------------------------------------.---------------------------------.
| Filename: MetaMCSandBox.h                | Meta Monte Carlo SandBox        |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2012 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_
# define LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "MCAlgorithm.h"
# include "MCOptimizer.h"
# include "../../../src/Luape/Function/ObjectLuapeFunctions.h"
# include <fstream>

namespace lbcpp
{

/*
** Luape functions
*/
class MCAlgorithmLuapeFunction : public UnaryObjectLuapeFunction<MCAlgorithmLuapeFunction>
{
public:
  MCAlgorithmLuapeFunction() : UnaryObjectLuapeFunction<MCAlgorithmLuapeFunction>(mcAlgorithmClass) {}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return mcAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const = 0;

  Variable computeObject(const ObjectPtr& object) const
    {return createAlgorithm(object.staticCast<MCAlgorithm>());} 
};

class IterateMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return iterateMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new IterateMCAlgorithm(input, 4);}
};

class StepMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return stepByStepMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new StepByStepMCAlgorithm(input, true);}
};

class LookAheadMCAlgorithmLuapeFunction : public MCAlgorithmLuapeFunction
{
public:
 virtual TypePtr initialize(const TypePtr* inputTypes)
    {return lookAheadMCAlgorithmClass;}

  virtual MCAlgorithmPtr createAlgorithm(const MCAlgorithmPtr& input) const
    {return new LookAheadMCAlgorithm(input);}
};

/////////////////

class CacheAndFiniteBudgetMCObjective : public MCObjective
{
public:
  CacheAndFiniteBudgetMCObjective(MCObjectivePtr objective, size_t budget, bool useCache)
    : objective(objective), budget(budget), useCache(useCache), numEvaluations(0), numCachedEvaluations(0), bestScoreSoFar(-DBL_MAX), nextCurvePoint(1) {}

  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
  {
    ++numEvaluations;
    LuapeNodeBuilderStatePtr builder = finalState.staticCast<LuapeNodeBuilderState>();
    if (builder->getStackSize() != 1)
      return -DBL_MAX;
    LuapeNodePtr node = builder->getStackElement(0);
    if (useCache)
    {
      std::map<LuapeNodePtr, double>::iterator it = cache.find(node);
      if (it != cache.end())
      {
        ++numCachedEvaluations;
        return it->second;
      }
    }
    //context.informationCallback(finalState->toShortString());
    double res = objective->evaluate(context, finalState);
    bestScoreSoFar = juce::jmax(res, bestScoreSoFar);

    if (numEvaluations == nextCurvePoint)
    {
      curve.push_back(bestScoreSoFar);
      nextCurvePoint *= 2;
    }

    if (useCache)
      cache[node] = res;
    return res;
  }

  virtual bool shouldStop() const
    {return numEvaluations >= budget || numCachedEvaluations >= 100 * budget;}

  size_t getNumEvaluations() const
    {return numEvaluations;}

  size_t getNumCachedEvaluations() const
    {return numCachedEvaluations;}

  const std::vector<double>& getCurve() const
    {return curve;}

protected:
  MCObjectivePtr objective;
  size_t budget;
  bool useCache;
  size_t numEvaluations;
  size_t numCachedEvaluations;
  std::map<LuapeNodePtr, double> cache;

  double bestScoreSoFar;
  size_t nextCurvePoint;
  std::vector<double> curve;
};

typedef ReferenceCountedObjectPtr<CacheAndFiniteBudgetMCObjective> CacheAndFiniteBudgetMCObjectivePtr;

class MetaMCSandBox : public WorkUnit
{
public:
  MetaMCSandBox() : budget(1024), complexity(8), numRuns(100) {}

  void testToto(ExecutionContext& context)
  {
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics();
    for (size_t i = 0; i < 1; ++i)
    {
      LuapeRegressorPtr problem = createProblem(context, 1);

      SymbolicRegressionMCObjectivePtr objective = new SymbolicRegressionMCObjective(problem);
      LuapeUniversePtr universe = problem->getUniverse();
      // x2 + x
      LuapeNodePtr x = problem->getInput(0);
      LuapeNodePtr node = new LuapeFunctionNode(addDoubleLuapeFunction(),
          new LuapeFunctionNode(mulDoubleLuapeFunction(), x, x), x);
      double score = objective->evaluate(context, problem, node);
      //stats->push(-score);
      context.informationCallback(String(-score));
    }
    //context.resultCallback("stats", stats);
    //context.informationCallback(stats->toShortString());
  }

  static void enumerateExhaustively(ExecutionContext& context, LuapeNodeBuilderStatePtr state, const String& description, std::ostream& ostr)
{
  if (state->isFinalState() && state->getStackSize() == 1)
  {
    ostr << description << std::endl;
    
    //LuapeNodePtr node = state->getStackElement(0);
    //res.push_back(node);
    //if (verbose)
    //  context.informationCallback(node->toShortString());
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
      String newDescription = description.isEmpty() ? String::empty : description + T(" ");
      newDescription += action.toShortString();
      enumerateExhaustively(context, state, newDescription, ostr);
      state->undoTransition(context, stateBackup);
    }
  }
}

  void testTutu(ExecutionContext& context)
  {
    for (size_t complexity = 1; complexity <= 8; ++complexity)
    {
      LuapeRegressorPtr problem = createProblem(context, 1);
      LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = problem->getSearchSpace(context, complexity, true);
      LuapeNodeBuilderStatePtr state = new LuapeNodeBuilderState(problem, typeSearchSpace);
      std::ofstream ostr((const char* )context.getFile("allrpn" + String((int)complexity) + T(".txt")).getFullPathName());
      enumerateExhaustively(context, state, String::empty, ostr);
    }
  }

  struct AlgorithmObjective : public MCBanditPoolObjective
  {
    AlgorithmObjective(MetaMCSandBox* owner) : owner(owner) {}

    MetaMCSandBox* owner;
    
    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 1.0; best = 0.0;}

    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      MCAlgorithmPtr algorithm = parameter.getObjectAndCast<MCAlgorithm>();
      size_t problemNumber = (size_t)(1 + (instanceIndex % 8));
      return owner->testAlgorithm(context, algorithm, false, problemNumber);
    }
  };

  virtual Variable run(ExecutionContext& context)
  {
    //testTutu(context);
    //return true;
    //testToto(context);
    //return true;
    std::vector<std::pair<String, MCAlgorithmPtr> > algorithms;
    generateMCAlgorithms(context, algorithms);
    
    algorithms.push_back(std::make_pair("RAND", rollout()));
    
    algorithms.push_back(std::make_pair("LA1", step(lookAhead(rollout()))));
    algorithms.push_back(std::make_pair("LA2", step(lookAhead(lookAhead(rollout())))));
    algorithms.push_back(std::make_pair("LA3", step(lookAhead(lookAhead(lookAhead(rollout()))))));
    algorithms.push_back(std::make_pair("LA4", step(lookAhead(lookAhead(lookAhead(lookAhead(rollout())))))));
    algorithms.push_back(std::make_pair("LA5", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout()))))))));
    algorithms.push_back(std::make_pair("LA6", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout())))))))));
    
    algorithms.push_back(std::make_pair("NMC2", step(lookAhead(step(lookAhead(rollout()))))));
    algorithms.push_back(std::make_pair("NMC3", step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))));
    algorithms.push_back(std::make_pair("NMC4", step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))))));
    algorithms.push_back(std::make_pair("NMC5", step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))))))));
    
    algorithms.push_back(std::make_pair("LA2bis", step(step(lookAhead(lookAhead(rollout()))))));
    algorithms.push_back(std::make_pair("LA2ter", step(step(step(lookAhead(lookAhead(rollout())))))));
    
    
    //algorithms.push_back(std::make_pair("LA7", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout()))))))))));
    //algorithms.push_back(std::make_pair("LA8", step(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(lookAhead(rollout())))))))))));
    //algorithms.push_back(std::make_pair("NMC6", step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(step(lookAhead(rollout()))))))))))))));
    
    context.informationCallback(String((int)algorithms.size()) + T(" Algorithms"));

    MCBanditPoolPtr pool = new MCBanditPool(new AlgorithmObjective(this), 5.0);
    pool->reserveArms(algorithms.size());
    for (size_t i = 0; i < algorithms.size(); ++i)
      if (!algorithms[i].second.isInstanceOf<IterateMCAlgorithm>())
        pool->createArm(algorithms[i].second);

    context.informationCallback(String((int)pool->getNumArms()) + T(" Arms"));
    pool->playIterations(context, 1000, 1000);
    return true;

    for (size_t problemNumber = 1; problemNumber <= 8; ++problemNumber)
    {
      context.enterScope(T("Problem ") + String((int)problemNumber));
      double bestScore = DBL_MAX;
      String bestAlgorithm;
      std::vector< std::vector<ScalarVariableStatistics> > resultCurves(algorithms.size());
      for (size_t i = 0; i < algorithms.size(); ++i)
      {
        context.enterScope(algorithms[i].first);
        context.resultCallback(T("algorithm"), i);
        context.resultCallback(T("algorithmName"), algorithms[i].first);

        MCAlgorithmPtr algorithm = algorithms[i].second;
        double score = testAlgorithm(context, algorithm, false, problemNumber, &resultCurves[i]);
        if (score < bestScore)
          bestScore = score, bestAlgorithm = algorithms[i].first;
        context.leaveScope(score);
      }
      context.informationCallback(T("Best: ") + bestAlgorithm + T(": ") + String(bestScore));

      context.enterScope(T("Results")); // skip first points
      for (size_t i = 3; i < resultCurves[0].size(); ++i)
      {
        context.enterScope(T("Budget ") + String(pow(2.0, (double)i)));
        context.resultCallback("logBudget", i);
        for (size_t j = 0; j < resultCurves.size(); ++j)
        {
          if (i < resultCurves[j].size())
          {
            double value = resultCurves[j][i].getMean();
            if (isNumberValid(value))
              context.resultCallback(algorithms[j].first, value);
          }
        }
        context.leaveScope();
      }
      context.leaveScope();

      context.leaveScope(bestAlgorithm);
    }
    return true;
  }

protected:
  friend class MetaMCSandBoxClass;

  size_t budget;
  size_t complexity;
  size_t numRuns;

  double testAlgorithm(ExecutionContext& context, MCAlgorithmPtr algorithm, bool useCache, size_t problemNumber, std::vector<ScalarVariableStatistics>* resultCurve = NULL)
  {
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("score");
    std::map<String, size_t> solutions;
    for (size_t i = 0; i < numRuns; ++i)
    {
      //context.enterScope(T("Iteration ") + String((int)i));
      //context.resultCallback("i", i);
      LuapeRegressorPtr problem = createProblem(context, problemNumber);
      LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = problem->getSearchSpace(context, complexity);
      CacheAndFiniteBudgetMCObjectivePtr objective = new CacheAndFiniteBudgetMCObjective(new SymbolicRegressionMCObjective(problem), budget, useCache);
      DecisionProblemStatePtr finalState;

      double score = iterate(algorithm, 0)->search(context, objective, new LuapeNodeBuilderState(problem, typeSearchSpace), NULL, finalState);
      if (finalState)
      {
        LuapeNodePtr formula = finalState.staticCast<LuapeNodeBuilderState>()->getStackElement(0);
        solutions[formula->toShortString()]++;
        stats->push(-score);

        // merge curve
        if (resultCurve)
        {
          const std::vector<double>& curve = objective->getCurve();
          if (curve.size() > resultCurve->size())
            resultCurve->resize(curve.size());
          for (size_t i = 0; i < curve.size(); ++i)
            (*resultCurve)[i].push(-curve[i]);
        }
      }

      /*
        context.informationCallback(finalState->toShortString() + T(" => ") + String(score));
        context.informationCallback(String((int)objective->getNumEvaluations()) + T(" evaluations, ")
          + String((int)objective->getNumCachedEvaluations()) + T(" cached evaluations"));
          */
      //context.leaveScope(score);

      if (numRuns > 1)
        context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }
    if (numRuns > 1)
    {
      context.resultCallback(T("scoreStats"), stats);
      context.informationCallback("Score: " + stats->toShortString());
    }

    // sort and display solutions
    /*
    std::multimap<size_t, String> sortedSolutions;
    for (std::map<String, size_t>::const_iterator it = solutions.begin(); it != solutions.end(); ++it)
      sortedSolutions.insert(std::make_pair(it->second, it->first));
    for (std::multimap<size_t, String>::reverse_iterator it = sortedSolutions.rbegin(); it != sortedSolutions.rend(); ++it)
    {
      String info = it->second;
      if (it->first > 1)
        info += T(" (") + String((int)it->first) + T(" times)");
      context.informationCallback(info);
    }*/

    return stats->getMean();
  }

  void generateMCAlgorithms(ExecutionContext& context, std::vector<std::pair<String, MCAlgorithmPtr> >& res)
  {
    LuapeInferencePtr problem = new LuapeInference();

    problem->addConstant(rollout());
    problem->addFunction(new IterateMCAlgorithmLuapeFunction());
    problem->addFunction(new StepMCAlgorithmLuapeFunction());
    problem->addFunction(new LookAheadMCAlgorithmLuapeFunction());
    problem->addTargetType(mcAlgorithmClass);

    std::vector<LuapeNodePtr> nodes;
    problem->enumerateNodesExhaustively(context, 7, nodes, true);

    res.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      LuapeNodePtr node = nodes[i];
      MCAlgorithmPtr algorithm = node->compute(context).getObjectAndCast<MCAlgorithm>();
      res.push_back(std::make_pair(algorithm->toShortString(), algorithm));
      //context.informationCallback(algorithm->toShortString());
    }
  }

  LuapeRegressorPtr createProblem(ExecutionContext& context, size_t problemNumber) const
  {
    LuapeRegressorPtr regressor = new LuapeRegressor();

    regressor->addInput(doubleType, "x");

    regressor->addConstant(1.0);

    regressor->addFunction(logDoubleLuapeFunction());
    regressor->addFunction(expDoubleLuapeFunction());
    regressor->addFunction(sinDoubleLuapeFunction());
    regressor->addFunction(cosDoubleLuapeFunction());

    regressor->addFunction(addDoubleLuapeFunction());
    regressor->addFunction(subDoubleLuapeFunction());
    regressor->addFunction(mulDoubleLuapeFunction());
    regressor->addFunction(divDoubleLuapeFunction());

    RandomGeneratorPtr random = context.getRandomGenerator();
    std::vector<ObjectPtr> examples(20);

    double lowerLimit = -1.0;
    double upperLimit = 1.0;
    if (problemNumber == 7)
      lowerLimit = 0.0, upperLimit = 2.0;
    else if (problemNumber == 8)
      lowerLimit = 0.0, upperLimit = 4.0;

    for (size_t i = 0; i < examples.size(); ++i)
    {
      double x = lowerLimit + (upperLimit - lowerLimit) * i / (examples.size() - 1.0);// random->sampleDouble(lowerLimit, upperLimit);
      double y = computeFunction(problemNumber, x);
      examples[i] = new Pair(new DenseDoubleVector(singletonEnumeration, doubleType, 1, x), y);
    }
    regressor->setSamples(context, examples);
    return regressor;
  }

  static double computeFunction(size_t problemNumber, double x)
  {
    double x2 = x * x;
    switch (problemNumber)
    {
    case 1: return x * x2 + x2 + x;
    case 2: return x2 * x2 + x * x2 + x2 + x;
    case 3: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
    case 4: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
    case 5: return sin(x2) * cos(x) - 1.0;
    case 6: return sin(x) + sin(x + x2);
    case 7: return log(x + 1) + log(x2 + 1);
    case 8: return sqrt(x);
    default: jassert(false); return 0.0;
    };
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_
