/*-----------------------------------------.---------------------------------.
| Filename: MixtureSampler.h               | Mixture Sampler                 |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 20:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_
# define LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class MixtureSampler : public CompositeSampler
{
public:
  MixtureSampler(const DenseDoubleVectorPtr& probabilities, const std::vector<SamplerPtr>& samplers)
    : CompositeSampler(samplers), probabilities(probabilities) {}
  MixtureSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    jassert(probabilities->getNumValues() == samplers.size());
    size_t index = random->sampleWithProbabilities(probabilities->getValues());
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    jassert(false);
    // FIXME: todo -> generic EM algorithm
  }

protected:
  friend class MixtureSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_
