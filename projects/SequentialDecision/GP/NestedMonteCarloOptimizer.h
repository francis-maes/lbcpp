/*-----------------------------------------.---------------------------------.
| Filename: NestedMonteCarloOptimizer.h    | Nested Monte Carlo Optimizer    |
| Author  : Francis Maes                   |                                 |
| Started : 26/09/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_NESTED_MONTE_CARLO_H_
# define LBCPP_OPTIMIZER_NESTED_MONTE_CARLO_H_

# include <lbcpp/Optimizer/Optimizer.h>

namespace lbcpp
{

class NestedMonteCarloOptimizerState : public OptimizerState
{
public:
};

typedef ReferenceCountedObjectPtr<NestedMonteCarloOptimizerState> NestedMonteCarloOptimizerStatePtr;

class NestedMonteCarloWorkUnit : public WorkUnit
{
public:
  NestedMonteCarloWorkUnit(DecisionProblemStatePtr initialState, FunctionPtr objective, size_t level)
    : initialState(initialState), objective(objective), level(level) {}
  NestedMonteCarloWorkUnit() {}

  virtual Variable run(ExecutionContext& context)
  {
    bestScore = DBL_MAX;
    bestFinalState = DecisionProblemStatePtr();
    nestedMonteCarlo(context, initialState, level);
    return new Pair(bestFinalState, bestScore);
  }

  double getBestScore() const
    {return bestScore;}

  const DecisionProblemStatePtr& getBestFinalState() const
    {return bestFinalState;}

protected:
  friend class NestedMonteCarloWorkUnitClass;

  DecisionProblemStatePtr initialState;
  FunctionPtr objective;
  size_t level;

  double bestScore;
  DecisionProblemStatePtr bestFinalState;

  void submitSolution(const DecisionProblemStatePtr& finalState, double score)
  {
    if (score < bestScore)
    {
      bestScore = score;
      bestFinalState = finalState;
    }
  }

  double nestedMonteCarlo(ExecutionContext& context, DecisionProblemStatePtr state, size_t level)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    double res = DBL_MAX;
    state = state->cloneAndCast<DecisionProblemState>();

    if (level == 0)
    {
      while (!state->isFinalState())
      {
        ContainerPtr actions = state->getAvailableActions();
        size_t n = actions->getNumElements();
        jassert(n);
        Variable action = actions->getElement(random->sampleSize(n));
        double reward;
        state->performTransition(context, action, reward);
      }
      res = objective->compute(context, state).toDouble();
      //context.informationCallback(state->toShortString() + T(" ==> ") + String(res));
      submitSolution(state, res);
    }
    else
    {
      while (!state->isFinalState())
      {
        ContainerPtr actions = state->getAvailableActions();
        size_t n = actions->getNumElements();
        jassert(n);
        Variable bestAction;
        double bestScore = DBL_MAX;
        for (size_t i = 0; i < n; ++i)
        {
          Variable action = actions->getElement(i);
          double reward;
          Variable stateBackup;
          state->performTransition(context, action, reward, &stateBackup);
          double score = nestedMonteCarlo(context, state, level - 1);
          if (score < bestScore)
          {
            bestScore = score;
            bestAction = action;
          }
          if (score < res)
            res = score;
          state->undoTransition(context, stateBackup);
        }
        if (!bestAction.exists())
          bestAction = actions->getElement(random->sampleSize(n));

        double reward;
        state->performTransition(context, bestAction, reward);
      }
      double score = objective->compute(context, state).toDouble();
      submitSolution(state, res);
      if (score < res)
        res = score;
    }
    return res;
  }
};

class NestedMonteCarloOptimizer : public Optimizer
{
public:
  NestedMonteCarloOptimizer(DecisionProblemStatePtr initialState, size_t level, size_t numIterations)
    : initialState(initialState), level(level), numIterations(numIterations) {}
  NestedMonteCarloOptimizer() : level(0), numIterations(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    FunctionPtr objective = problem->getObjective();

    if (numIterations > 1)
    {
      CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Nested Monte Carlo runs"), numIterations);
      for (size_t i = 0; i < numIterations; ++i)
        workUnit->setWorkUnit(i, new NestedMonteCarloWorkUnit(initialState, objective, level));
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
      WorkUnitPtr workUnit = new NestedMonteCarloWorkUnit(initialState, objective, level);
      PairPtr best = workUnit->run(context).getObjectAndCast<Pair>();
      optimizerState->submitSolution(best->getFirst(), best->getSecond().toDouble());
    }
    return optimizerState;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new NestedMonteCarloOptimizerState();}

protected:
  friend class NestedMonteCarloOptimizerClass;

  DecisionProblemStatePtr initialState;
  size_t level;
  size_t numIterations;
};

};/* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_NESTED_MONTE_CARLO_H_
