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

  virtual void optimize(ExecutionContext& context)
  {
    SearchStatePtr state = trajectory->getFinalState();
    DiscreteDomainPtr actions = state->getActionDomain();
    size_t n = actions->getNumElements();
#if 0
    state = state->cloneAndCast<DecisionProblemState>();
    std::vector<Variable> actions(previousActions);
//    context.informationCallback(T("----"));
    while (!state->isFinalState() && !objective->shouldStop())
    {
      std::vector<Variable> bestActions(actions);
      DecisionProblemStatePtr bestFinalState;
      /*double score = */subSearch(context, objective, state, bestActions, bestFinalState);

      /*if (bestFinalState)
      {
        String info = "Best Actions:";
        for (size_t i = 0; i < bestActions.size(); ++i)
          info += " " + bestActions[i].toShortString();
        context.informationCallback(info + T(" (") + String(score) + T(")"));
      }
      else
        context.informationCallback("No Best Final State");*/

      // select action
      Variable selectedAction;
      if (useGlobalBest && this->bestFinalState)
        selectedAction = this->bestActions[actions.size()];  // global best
      else if (bestFinalState)
        selectedAction = bestActions[actions.size()];       // local best
      if (!selectedAction.exists())
        break;

      double reward;
      actions.push_back(selectedAction);
      state->performTransition(context, selectedAction, reward);
      
      while (!state->isFinalState())
      {
        ContainerPtr availableActions = state->getAvailableActions();
        if (availableActions->getNumElements() > 1)
          break;
        Variable action = availableActions->getElement(0);
        actions.push_back(action);
        state->performTransition(context, action, reward);
      }
    }
#endif // 0
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_ALGORITHM_STEP_H_
