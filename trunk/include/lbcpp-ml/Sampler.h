/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler                         |
| Author  : Francis Maes                   |  (represents a distribution)    |
| Started : 22/09/2012 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SAMPLER_H_
# define LBCPP_ML_SAMPLER_H_

# include "Domain.h"

namespace lbcpp
{

class Sampler : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain) = 0;

  virtual ObjectPtr sample(ExecutionContext& context) const = 0;
  virtual bool isDeterministic() const // returns true if the sampler has became deterministic
    {return false;}

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {jassertfalse;}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
    {jassertfalse;}
};

extern SamplerPtr uniformContinuousSampler();
extern SamplerPtr diagonalGaussianSampler();
extern SamplerPtr diagonalGaussianDistributionSampler();

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SAMPLER_H_
