/*-----------------------------------------.---------------------------------.
| Filename: BeamSearchOptimizer.h          | Beam Search Optimizer           |
| Author  : Francis Maes                   |                                 |
| Started : 27/09/2011 12:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BEAM_SEARCH_OPTIMIZER_H_
# define LBCPP_SEQUENTIAL_DECISION_BEAM_SEARCH_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include "../Core/DecisionProblem.h"

namespace lbcpp
{

class BeamSearchOptimizerState : public OptimizerState
{
public:
  BeamSearchOptimizerState(DecisionProblemStatePtr initialState, size_t beamSize)
    : beamSize(beamSize)
    {currentStates.insert(initialState);}
  BeamSearchOptimizerState() {}

  Variable doIteration(ExecutionContext& context, size_t iteration, const OptimizationProblemPtr& problem)
  {
    FunctionPtr objective = problem->getObjective();
    SortedStateMap successorStates;

    for (StateSet::const_iterator it = currentStates.begin(); it != currentStates.end(); ++it)
    {
      DecisionProblemStatePtr state = *it;
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable action = actions->getElement(i);
        DecisionProblemStatePtr newState = state->cloneAndCast<DecisionProblemState>();
        double reward;
        newState->performTransition(context, action, reward);
        
        double score = objective->compute(context, newState).toDouble();
        if (newState->isFinalState())
        {
          submitSolution(newState, score);
          if (score < DBL_MAX)
            finalStates.insert(std::make_pair(score, newState));
          context.informationCallback(T("Final state: ") + newState->toShortString() + T(" [") + String(score) + T("]"));
        }
        else
          successorStates.insert(std::make_pair(score, newState));
      }
    }

    double bestIterationScore = DBL_MAX;
    DecisionProblemStatePtr bestIterationSolution;
    currentStates.clear();
    if (successorStates.size())
    {
      bestIterationScore = successorStates.begin()->first;
      bestIterationSolution = successorStates.begin()->second;

      size_t i = 0;
      for (SortedStateMap::const_iterator it = successorStates.begin(); it != successorStates.end() && i < beamSize; ++it, ++i)
      {
        if (it->first == DBL_MAX)
          break;
        context.informationCallback(T("Succesor: ") + it->second->toShortString() + T(" [") + String(it->first) + T("]"));
        currentStates.insert(it->second);
      }
    }

    return finishIteration(context, problem, iteration, bestIterationScore, bestIterationSolution);
  }

  typedef std::multimap<double, DecisionProblemStatePtr> SortedStateMap;
  typedef std::set<DecisionProblemStatePtr> StateSet;

  size_t getNumCurrentStates() const
    {return currentStates.size();}

  const SortedStateMap& getFinalStates() const
    {return finalStates;}

protected:
  friend class BeamSearchOptimizerStateClass;

  size_t beamSize;
  StateSet currentStates;
  SortedStateMap finalStates;
};

typedef ReferenceCountedObjectPtr<BeamSearchOptimizerState> BeamSearchOptimizerStatePtr;

class BeamSearchOptimizer : public Optimizer
{
public:
  BeamSearchOptimizer(DecisionProblemStatePtr initialState, size_t beamSize = 100)
    : initialState(initialState), beamSize(beamSize) {}
  BeamSearchOptimizer() : beamSize(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    BeamSearchOptimizerStatePtr state = optimizerState.staticCast<BeamSearchOptimizerState>();
    FunctionPtr objective = problem->getObjective();

    for (size_t iteration = 0; state->getNumCurrentStates() > 0; ++iteration)
    {
      context.enterScope(T("Iteration ") + String((int)iteration));
      Variable res = state->doIteration(context, iteration, problem);
      context.leaveScope(res);
    }

    return state;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new BeamSearchOptimizerState(initialState, beamSize);}

protected:
  friend class BeamSearchOptimizerClass;

  DecisionProblemStatePtr initialState;
  size_t beamSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BEAM_SEARCH_OPTIMIZER_H_
