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
  virtual ObjectPtr sampleAction(ExecutionContext& context, SearchStatePtr state) const
  {
    DiscreteDomainPtr actionDomain = state->getActionDomain().staticCast<DiscreteDomain>();
    size_t n = actionDomain->getNumElements();
    jassert(n > 0);
    return actionDomain->getElement(context.getRandomGenerator()->sampleSize(n));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SAMPLER_SEARCH_RANDOM_H_
