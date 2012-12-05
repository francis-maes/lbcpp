/*-----------------------------------------.---------------------------------.
| Filename: UniformScalarVectorSampler.h   | Sample uniformly in R^n         |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 11:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SAMPLER_UNIFORM_CONTINUOUS_H_
# define ML_SAMPLER_UNIFORM_CONTINUOUS_H_

# include <ml/Sampler.h>

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

#endif // !ML_SAMPLER_UNIFORM_CONTINUOUS_H_
