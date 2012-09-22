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

class MOOSampler : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const MOODomainPtr& domain) = 0;

  virtual ObjectPtr sample(ExecutionContext& context) const = 0;
  virtual bool isDegenerate() const // returns true if the sampler has became deterministic
    {return false;}

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects) = 0;
  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object) = 0;
};

extern MOOSamplerPtr uniformContinuousSampler();
extern MOOSamplerPtr diagonalGaussianSampler();

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SAMPLER_H_
