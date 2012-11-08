/*-----------------------------------------.---------------------------------.
| Filename: LookAheadSearchAlgorithm.h     | Look-ahead Search Algorithm     |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SEARCH_ALGORITHM_LOOK_AHEAD_H_
# define LBCPP_ML_SEARCH_ALGORITHM_LOOK_AHEAD_H_

# include <lbcpp-ml/Search.h>

namespace lbcpp
{

class LookAheadSearchAlgorithm : public DecoratorSearchAlgorithm
{
public:
  LookAheadSearchAlgorithm(SearchAlgorithmPtr algorithm, double numActions = 1.0)
    : DecoratorSearchAlgorithm(algorithm), numActions(numActions) {}
  LookAheadSearchAlgorithm() : numActions(0.0) {}

protected:
  friend class LookAheadSearchAlgorithmClass;

  double numActions;

  virtual void runSolver(ExecutionContext& context)
  {
    SearchStatePtr state = trajectory->getFinalState();
    DiscreteDomainPtr actions = state->getActionDomain();
    size_t n = actions->getNumElements();
    if (n == 0)
      return;

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(n, order);
    std::vector<ObjectPtr> selectedActions((size_t)(juce::jmax(1.0, n * numActions)));
    for (size_t i = 0; i < selectedActions.size(); ++i)
      selectedActions[i] = actions->getElement(order[i]);

    for (size_t i = 0; i < selectedActions.size(); ++i)
    {
      if (callback->shouldStop())
        break;
      ObjectPtr action = selectedActions[i];
      Variable stateBackup;
      state->performTransition(context, action, &stateBackup);
      trajectory->append(action);

      subSearch(context);

      trajectory->pop();      
      state->undoTransition(context, stateBackup);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_ALGORITHM_LOOK_AHEAD_H_
