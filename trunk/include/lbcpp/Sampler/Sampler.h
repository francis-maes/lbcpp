/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler base class              |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_H_
# define LBCPP_SAMPLER_H_

# include "../Data/RandomGenerator.h"
# include "../Core/Variable.h"

namespace lbcpp
{ 
  
class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

class Sampler : public Object
{
public:
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const = 0;

  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset) = 0;

protected:
  friend class SamplerClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_
