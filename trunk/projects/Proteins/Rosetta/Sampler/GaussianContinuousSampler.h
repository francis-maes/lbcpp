/*-----------------------------------------.---------------------------------.
| Filename: GaussianContinuousSampler.h    | GaussianContinuousSampler       |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  10:56:36         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_GAUSSIAN_CONTINUOUS_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_GAUSSIAN_CONTINUOUS_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

namespace lbcpp
{

class GaussianContinuousSampler;
typedef ReferenceCountedObjectPtr<GaussianContinuousSampler> GaussianContinuousSamplerPtr;

class GaussianContinuousSampler : public ContinuousSampler
{
public:
  GaussianContinuousSampler()
    : ContinuousSampler()
  {
  }

  GaussianContinuousSampler(double mean, double std)
    : ContinuousSampler(mean, std)
  {
  }

  GaussianContinuousSampler(const GaussianContinuousSampler& sampler)
    : ContinuousSampler(sampler.mean, sampler.std)
  {
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    return Variable(random->sampleDoubleFromGaussian(mean, std));
  }

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */
  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() < 2)
      return;
    double temporaryMean = getMean(dataset);
    double temporaryVariance = getVariance(dataset, temporaryMean);
    if (temporaryVariance <= 0)
      temporaryVariance = juce::jmax(1.0, std::abs(temporaryMean * 0.3));

    mean = temporaryMean;
    std = std::sqrt(temporaryVariance) >= std::pow(10.0, -5) ? std::sqrt(temporaryVariance)
        : std::pow(10.0, -5);
  }

protected:
  friend class GaussianContinuousSamplerClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_GAUSSIAN_CONTINUOUS_SAMPLER_H_
