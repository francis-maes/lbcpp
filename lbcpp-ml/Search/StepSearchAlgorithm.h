/*-----------------------------------------.---------------------------------.
| Filename: StepSearchAlgorithm.h          | Step Search Algorithm           |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SEARCH_ALGORITHM_STEP_H_
# define LBCPP_ML_SEARCH_ALGORITHM_STEP_H_

# include <lbcpp-ml/Search.h>
# include <lbcpp-ml/SolutionContainer.h>

namespace lbcpp
{

class StepSearchAlgorithm : public DecoratorSearchAlgorithm
{
public:
  StepSearchAlgorithm(SearchAlgorithmPtr algorithm)
    : DecoratorSearchAlgorithm(algorithm) {}
  StepSearchAlgorithm() {}

protected:
  friend class StepSearchAlgorithmClass;

  virtual void runSolver(ExecutionContext& context)
  {
    SearchStatePtr state = trajectory->getFinalState();
    DiscreteDomainPtr actions = state->getActionDomain();
    size_t n = actions->getNumElements();

    SolverCallbackPtr previousCallback = callback;
    ParetoFrontPtr front = new ParetoFront();
    callback = compositeSolverCallback(fillParetoFrontSolverCallback(front), previousCallback);

    while (!state->isFinalState() && !callback->shouldStop())
    {
      subSearch(context);

      SearchTrajectoryPtr bestTrajectory = front->getSolution(0).staticCast<SearchTrajectory>();
      if (!bestTrajectory)
        break;
      ObjectPtr selectedAction = bestTrajectory->getAction(trajectory->getLength());

#ifdef JUCE_DEBUG
      for (size_t i = 0; i < trajectory->getLength(); ++i)
        jassert(trajectory->getAction(i) == bestTrajectory->getAction(i));
#endif // JUCE_DEBUG

      state->performTransition(context, selectedAction);
      trajectory->append(selectedAction);

#if 0
      while (!state->isFinalState())
      {
        DiscreteDomainPtr availableActions = state->getActionDomain().staticCast<DiscreteDomain>();
        if (availableActions->getNumElements() != 1)
          break;
        ObjectPtr action = availableActions->getElement(0);

        state->performTransition(context, action);
        trajectory->append(action);
      }
#endif // 0
    }
    callback = previousCallback;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_ALGORITHM_STEP_H_
