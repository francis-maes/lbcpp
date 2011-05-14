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
  DiscretizeSampler()
    : Sampler(), minValue(0), maxValue(1)
  {
  }

  DiscretizeSampler(int minValue, int maxValue, ContinuousSamplerPtr& sampler)
    : Sampler(), minValue(minValue), maxValue(maxValue)
  {
    this->sampler = sampler->cloneAndCast<ContinuousSampler>();
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    double sampled = juce::jlimit((double)minValue, (double)maxValue, sampler->sample(context,
        random, inputs).getDouble());
    int roundDown = std::floor(sampled);
    int roundUp = std::ceil(sampled);
    double downGap = sampled - roundDown;
    double upGap = roundUp - sampled;
    if (downGap < upGap)
      return roundDown;
    else
      return roundUp;
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    std::vector<Variable> learning(dataset.size());
    for (int i = 0; i < dataset.size(); i++)
      learning[i] = Variable(dataset[i].toDouble());
    sampler->learn(context, dataset);
  }

protected:
  friend class DiscretizeSamplerClass;
  int minValue;
  int maxValue;
  ContinuousSamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_DISCREIZE_SAMPLER_H_
