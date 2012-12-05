/*-----------------------------------------.---------------------------------.
| Filename: BinaryMixtureSampler.h         | Binary Mixture Sampler          |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 21:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SAMPLER_BINARY_MIXTURE_H_
# define ML_SAMPLER_BINARY_MIXTURE_H_

# include <ml/Sampler.h>

namespace lbcpp
{

class BinaryMixtureSampler : public Sampler
{
public:
  BinaryMixtureSampler(SamplerPtr sampler1, SamplerPtr sampler2, double probability)
    : sampler1(sampler1), sampler2(sampler2), probability(probability) {}
  BinaryMixtureSampler() : probability(0.5) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    sampler1->initialize(context, domain);
    sampler2->initialize(context, domain);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return (context.getRandomGenerator()->sampleBool(probability) ? sampler2 : sampler1)->sample(context);}

protected:
  friend class BinaryMixtureSamplerClass;

  SamplerPtr sampler1;
  SamplerPtr sampler2;
  double probability;
};

}; /* namespace lbcpp */

#endif // !ML_SAMPLER_BINARY_MIXTURE_H_
