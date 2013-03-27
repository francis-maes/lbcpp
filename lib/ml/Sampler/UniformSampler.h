/*-----------------------------------------.---------------------------------.
| Filename: UniformSampler.h               | Sample uniformly in the domain  |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 11:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SAMPLER_UNIFORM_H_
# define ML_SAMPLER_UNIFORM_H_

# include <ml/Sampler.h>

namespace lbcpp
{

class UniformSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain;}

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return domain->sampleUniformly(context.getRandomGenerator());}

protected:
  DomainPtr domain;
};

}; /* namespace lbcpp */

#endif // !ML_SAMPLER_UNIFORM_H_
