/*-----------------------------------------.---------------------------------.
| Filename: RejectionSampler.h             | Rejection Sampler               |
| Author  : Francis Maes                   |                                 |
| Started : 20/05/2011 12:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_REJECTION_H_
# define LBCPP_SAMPLER_COMPOSITE_REJECTION_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Sampler/Sampler.h>
 
namespace lbcpp
{

class RejectionSampler : public DecoratorSampler
{
public:
  RejectionSampler(SamplerPtr sampler, FunctionPtr predicate)
    : DecoratorSampler(sampler), predicate(predicate) {}
  RejectionSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    while (true)
    {
      Variable candidate = sampler->sample(context, random, inputs);
      if (predicate->compute(context, candidate).getBoolean())
        return candidate;
    }
    jassert(false);
    return Variable();
  }

  // FIXME: learn and other stuffs are not implemented

protected:
  friend class RejectionSamplerClass;

  FunctionPtr predicate;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_REJECTION_H_
