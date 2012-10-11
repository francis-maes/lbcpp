/*-----------------------------------------.---------------------------------.
| Filename: RolloutSearchAlgorithm.h       | Rollout Search Algorithm        |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SEARCH_ALGORITHM_ROLLOUT_H_
# define LBCPP_ML_SEARCH_ALGORITHM_ROLLOUT_H_

# include <lbcpp-ml/Search.h>

namespace lbcpp
{

class RolloutSearchAlgorithm : public SearchAlgorithm
{
public:
  virtual void optimize(ExecutionContext& context)
  {
    SearchStatePtr state = trajectory->getFinalState();
    while (!state->isFinalState())
    {
      if (problem->shouldStop())
        return;
      DiscreteDomainPtr availableActions = state->getActionDomain().staticCast<DiscreteDomain>();
      size_t n = availableActions->getNumElements();
      if (!n)
        return;
      ObjectPtr action = availableActions->getElement(context.getRandomGenerator()->sampleSize(n));
      trajectory->append(action);
      state->performTransition(context, action);
    }
    trajectory->setFinalState(state);
    evaluate(context, trajectory);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_ALGORITHM_ROLLOUT_H_
