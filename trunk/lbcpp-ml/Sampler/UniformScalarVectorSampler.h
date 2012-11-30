/*-----------------------------------------.---------------------------------.
| Filename: UniformScalarVectorSampler.h     | Sample uniformly in R^n         |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 11:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SAMPLER_UNIFORM_CONTINUOUS_H_
# define LBCPP_MOO_SAMPLER_UNIFORM_CONTINUOUS_H_

# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class UniformScalarVectorSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain.staticCast<ScalarVectorDomain>(); jassert(this->domain);}

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return domain->sampleUniformly(context.getRandomGenerator());}

protected:
  ScalarVectorDomainPtr domain;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SAMPLER_UNIFORM_CONTINUOUS_H_
