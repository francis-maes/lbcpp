/*-----------------------------------------.---------------------------------.
| Filename: NestedMonteCarloOptimizer.h    | Nested Monte Carlo Optimizer    |
| Author  : Francis Maes                   |                                 |
| Started : 26/09/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_NESTED_MONTE_CARLO_H_
# define LBCPP_OPTIMIZER_NESTED_MONTE_CARLO_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

class NestedMonteCarloWorkUnit : public WorkUnit
{
public:
  NestedMonteCarloWorkUnit(const OptimizerStatePtr& optimizerState, size_t level)
    : optimizerState(optimizerState), level(level) {}
  NestedMonteCarloWorkUnit() {}

  virtual Variable run(ExecutionContext& context)
  {
    bestScore = optimizerState->getWorstScore();
    bestFinalState = DecisionProblemStatePtr();
    nestedMonteCarlo(context, optimizerState->getProblem()->getInitialState(), level);
    return new Pair(bestFinalState, bestScore);
  }

  double getBestScore() const
    {return bestScore;}

  const DecisionProblemStatePtr& getBestFinalState() const
    {return bestFinalState;}

protected:
  friend class NestedMonteCarloWorkUnitClass;

  OptimizerStatePtr optimizerState;
  size_t level;

  double bestScore;
  DecisionProblemStatePtr bestFinalState;

  void submitSolution(ExecutionContext& context, const DecisionProblemStatePtr& finalState, double score)
  {
    if (optimizerState->isScoreBetterThan(score, bestScore))
    {
      //context.informationCallback(finalState->toShortString() + T(" [") + String(score) + T("]"));
      bestScore = score;
      bestFinalState = finalState;
    }
  }

  double nestedMonteCarlo(ExecutionContext& context, DecisionProblemStatePtr state, size_t level)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    FunctionPtr objective = optimizerState->getObjective();

    double res = optimizerState->getWorstScore();
    state = state->cloneAndCast<DecisionProblemState>();

    if (level == 0)
    {
      while (!state->isFinalState())
      {
        ContainerPtr actions = state->getAvailableActions();
        size_t n = actions->getNumElements();
        if (!n)
          break;
        Variable action = actions->getElement(random->sampleSize(n));
        double reward;
        state->performTransition(context, action, reward);
      }
      res = objective->compute(context, state).toDouble();
      //context.informationCallback(state->toShortString() + T(" ==> ") + String(res));
      submitSolution(context, state, res);
    }
    else
    {
      while (!state->isFinalState())
      {
        ContainerPtr actions = state->getAvailableActions();
        size_t n = actions->getNumElements();
        if (!n)
          break;

        Variable bestAction;
        double bestScore = optimizerState->getWorstScore();
        for (size_t i = 0; i < n; ++i)
        {
          Variable action = actions->getElement(i);
          double reward;
          Variable stateBackup;
          state->performTransition(context, action, reward, &stateBackup);
          double score = nestedMonteCarlo(context, state, level - 1);
          if (optimizerState->isScoreBetterThan(score, bestScore))
          {
            bestScore = score;
            bestAction = action;
          }
          if (optimizerState->isScoreBetterThan(score, res))
            res = score;
          state->undoTransition(context, stateBackup);
        }
        if (!bestAction.exists())
          bestAction = actions->getElement(random->sampleSize(n));

        double reward;
        state->performTransition(context, bestAction, reward);
      }
      double score = objective->compute(context, state).toDouble();
      submitSolution(context, state, res);
      if (optimizerState->isScoreBetterThan(score, res))
        res = score;
    }
    return res;
  }
};

class NestedMonteCarloOptimizer : public Optimizer
{
public:
  NestedMonteCarloOptimizer(size_t level, size_t numIterations)
    : level(level), numIterations(numIterations) {}
  NestedMonteCarloOptimizer() : level(0), numIterations(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    if (false)//numIterations > 1) // parallel version
    {
      CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Nested Monte Carlo runs"), numIterations);
      for (size_t i = 0; i < numIterations; ++i)
        workUnit->setWorkUnit(i, new NestedMonteCarloWorkUnit(optimizerState, level));
      workUnit->setProgressionUnit(T("Runs"));
      ContainerPtr results = context.run(workUnit).getObjectAndCast<Container>();
      for (size_t i = 0; i < numIterations; ++i)
      {
        PairPtr best = results->getElement(i).getObjectAndCast<Pair>();
        optimizerState->submitSolution(best->getFirst(), best->getSecond().toDouble());
      }
    }
    else
    {
      // single-thread version
      for (size_t i = 0; i < numIterations; ++i)
      {
        WorkUnitPtr workUnit = new NestedMonteCarloWorkUnit(optimizerState, level);
        PairPtr best = workUnit->run(context).getObjectAndCast<Pair>();
        optimizerState->submitSolution(best->getFirst(), best->getSecond().toDouble());
      }
    }
    return optimizerState;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new OptimizerState(problem);}

protected:
  friend class NestedMonteCarloOptimizerClass;

  size_t level;
  size_t numIterations;
};

};/* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_NESTED_MONTE_CARLO_H_
