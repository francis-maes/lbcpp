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

namespace lbcpp
{

class CacheAndFiniteBudgetMCObjective : public MCObjective
{
public:
  CacheAndFiniteBudgetMCObjective(MCObjectivePtr objective, size_t budget, bool useCache)
    : objective(objective), budget(budget), useCache(useCache), numEvaluations(0), numCachedEvaluations(0) {}

  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
  {
    LuapeGraphBuilderStatePtr builder = finalState.staticCast<LuapeGraphBuilderState>();
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
    ++numEvaluations;
    double res = objective->evaluate(context, finalState);
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

protected:
  MCObjectivePtr objective;
  size_t budget;
  bool useCache;
  size_t numEvaluations;
  size_t numCachedEvaluations;
  std::map<LuapeNodePtr, double> cache;
};

typedef ReferenceCountedObjectPtr<CacheAndFiniteBudgetMCObjective> CacheAndFiniteBudgetMCObjectivePtr;

class MetaMCSandBox : public WorkUnit
{
public:
  MetaMCSandBox() : budget(1024), complexity(8), numRuns(100) {}

  double testAlgorithm(ExecutionContext& context, MCAlgorithmPtr algorithm, bool useCache, size_t problemNumber)
  {
    String algoName = algorithm->toShortString();
    if (useCache)
      algoName += T(" with cache");
    context.enterScope(algoName);
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("score");
    for (size_t i = 0; i < numRuns; ++i)
    {
      //context.enterScope(T("Iteration ") + String((int)i));
      //context.resultCallback("i", i);
      LuapeRegressorPtr problem = createProblem(context, problemNumber);
      LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace = problem->getSearchSpace(context, complexity);
      CacheAndFiniteBudgetMCObjectivePtr objective = new CacheAndFiniteBudgetMCObjective(new SymbolicRegressionMCObjective(problem), budget, useCache);
      DecisionProblemStatePtr finalState;
      double score = iterate(algorithm, 0)->search(context, objective, new LuapeGraphBuilderState(problem, typeSearchSpace), NULL, finalState);
      stats->push(-score);

      /*
        context.informationCallback(finalState->toShortString() + T(" => ") + String(score));
        context.informationCallback(String((int)objective->getNumEvaluations()) + T(" evaluations, ")
          + String((int)objective->getNumCachedEvaluations()) + T(" cached evaluations"));
          */
      //context.leaveScope(score);
    }
    context.informationCallback(stats->toShortString());
    context.leaveScope(stats);
    return stats->getMean();
  }

  virtual Variable run(ExecutionContext& context)
  {
    MCAlgorithmSet algorithmSet(3);
    std::vector<MCAlgorithmPtr> algorithms;
    algorithmSet.getAlgorithms(algorithms);
    context.informationCallback(String((int)algorithms.size()) + T(" Algorithms"));

    double bestScore = DBL_MAX;
    MCAlgorithmPtr bestAlgorithm;
    for (size_t i = 0; i < algorithms.size(); ++i)
    {
      MCAlgorithmPtr algorithm = algorithms[i];
      double score = testAlgorithm(context, algorithm, true, 0);
      if (score < bestScore)
        bestScore = score, bestAlgorithm = algorithm;
    }
    context.informationCallback(T("Best: ") + bestAlgorithm->toShortString() + T(": ") + String(bestScore));
    return true;
  }

protected:
  friend class MetaMCSandBoxClass;

  size_t budget;
  size_t complexity;
  size_t numRuns;

  LuapeRegressorPtr createProblem(ExecutionContext& context, size_t problemNumber) const
  {
    LuapeRegressorPtr regressor = new LuapeRegressor();

    regressor->addInput(doubleType, "x");

    RandomGeneratorPtr random = context.getRandomGenerator();
    std::vector<ObjectPtr> examples(20);
    for (size_t i = 0; i < examples.size(); ++i)
    {
      double x = random->sampleDouble(-1.0, 1.0);
      double y = x * x * x + x * x + x;
      examples[i] = new Pair(new DenseDoubleVector(singletonEnumeration, doubleType, 1, x), y);
    }
    regressor->setSamples(context, examples);

    // FIXME: 1  (constant)

    regressor->addFunction(addDoubleLuapeFunction());
    regressor->addFunction(subDoubleLuapeFunction());
    regressor->addFunction(mulDoubleLuapeFunction());
    regressor->addFunction(divDoubleLuapeFunction());     
    regressor->addFunction(logDoubleLuapeFunction());
    // FIXME: exp, sin, cos


    return regressor;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_META_MONTE_CARLO_SAND_BOX_H_
