/*-----------------------------------------.---------------------------------.
| Filename: RandomSearchSampler.h          | Random Search Sampler           |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2012 12:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SAMPLER_SEARCH_RANDOM_H_
# define ML_SAMPLER_SEARCH_RANDOM_H_

# include <ml/Search.h>
# include <ml/Sampler.h>

namespace lbcpp
{

class RandomSearchSampler : public SearchSampler
{
public:
  virtual ObjectPtr sampleAction(ExecutionContext& context, SearchTrajectoryPtr trajectory) const
  {
    SearchStatePtr state = trajectory->getFinalState();
    DiscreteDomainPtr actionDomain = state->getActionDomain().dynamicCast<DiscreteDomain>();
    if (actionDomain)
    {
      size_t n = actionDomain->getNumElements();
      jassert(n > 0);
      return actionDomain->getElement(context.getRandomGenerator()->sampleSize(n));
    }
    
    ScalarDomainPtr scalarDomain = state->getActionDomain().dynamicCast<ScalarDomain>();
    if (scalarDomain)
      return new Double(context.getRandomGenerator()->sampleDouble(scalarDomain->getLowerLimit(), scalarDomain->getUpperLimit()));
    
    jassertfalse;
    return ObjectPtr();
  }
};

}; /* namespace lbcpp */

#endif // !ML_SAMPLER_SEARCH_RANDOM_H_
