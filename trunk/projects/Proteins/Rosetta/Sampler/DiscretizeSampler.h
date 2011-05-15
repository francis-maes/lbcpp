/*-----------------------------------------.---------------------------------.
| Filename: DiscretizeSampler.h            | DiscretizeSampler               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 14 mai 2011  18:26:22          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_DISCREIZE_SAMPLER_H_
# define LBCPP_SAMPLER_DISCREIZE_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

namespace lbcpp
{

class DiscretizeSampler;
typedef ReferenceCountedObjectPtr<DiscretizeSampler> DiscretizeSamplerPtr;

class DiscretizeSampler : public Sampler
{
public:
  DiscretizeSampler(const ContinuousSamplerPtr& sampler, int minValue, int maxValue)
    : sampler(sampler), minValue(minValue), maxValue(maxValue)
  {
  }

  DiscretizeSampler() : minValue(0), maxValue(1) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    return juce::jlimit(minValue, maxValue, juce::roundDoubleToInt(sampler->sample(context, random,
        inputs).getDouble()));
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    std::vector<Variable> learning(dataset.size());
    for (size_t i = 0; i < dataset.size(); i++)
      learning[i] = Variable(dataset[i].toDouble());
    sampler->learn(context, learning);
  }

protected:
  friend class DiscretizeSamplerClass;

  ContinuousSamplerPtr sampler;
  int minValue;
  int maxValue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_DISCREIZE_SAMPLER_H_
