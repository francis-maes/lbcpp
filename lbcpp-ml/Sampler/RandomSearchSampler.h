/*-----------------------------------------.---------------------------------.
| Filename: RandomSearchSampler.h          | Random Search Sampler           |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2012 12:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SAMPLER_SEARCH_RANDOM_H_
# define LBCPP_ML_SAMPLER_SEARCH_RANDOM_H_

# include <lbcpp-ml/Search.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class RandomSearchSampler : public SearchSampler
{
public:
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    SearchTrajectoryPtr res(new SearchTrajectory());
    SearchStatePtr state = domain->createInitialState();
    while (!state->isFinalState())
    {
      ObjectPtr action = sampleAction(context, state);
      res->append(state->cloneAndCast<SearchState>(), action);
      state->performTransition(context, action);
    }
    res->setFinalState(state);
    return res;
  }

private:
  ObjectPtr sampleAction(ExecutionContext& context, const SearchStatePtr& state) const
  {
    DiscreteDomainPtr actionDomain = state->getActionDomain().staticCast<DiscreteDomain>();
    size_t n = actionDomain->getNumElements();
    jassert(n > 0);
    return actionDomain->getElement(context.getRandomGenerator()->sampleSize(n));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SAMPLER_SEARCH_RANDOM_H_
