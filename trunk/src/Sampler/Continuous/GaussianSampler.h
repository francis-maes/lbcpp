/*-----------------------------------------.---------------------------------.
| Filename: GaussianSampler.h              | Gaussian Sampler                |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  10:56:36         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_CONTINUOUS_GAUSSIAN_H_
# define LBCPP_SAMPLER_CONTINUOUS_GAUSSIAN_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class GaussianSampler;
typedef ReferenceCountedObjectPtr<GaussianSampler> GaussianSamplerPtr;

class GaussianSampler : public ContinuousSampler
{
public:
  GaussianSampler(double mean = 0.0, double stddev = 1.0)
    : mean(mean), stddev(stddev)
    {}

  virtual double computeExpectation(const Variable* inputs = NULL) const
    {return mean;}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleDoubleFromGaussian(mean, stddev);}

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */

#if 1
  // new
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    ScalarVariableMeanAndVariance v;
    for (size_t i = 0; i < dataset.size(); i++)
      v.push(dataset[i].getDouble());
    mean = v.getMean();
    stddev = v.getStandardDeviation();
  }

#else
  // old
  static void getMeanAndVariance(const std::vector<Variable>& dataset, double& mean, double& variance)
  {
    ScalarVariableMeanAndVariance v;
    for (size_t i = 0; i < dataset.size(); i++)
      v.push(dataset[i].getDouble());
    mean = v.getMean();
    variance = v.getVariance();
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 2)
      return;
    double temporaryMean;
    double temporaryVariance;
    getMeanAndVariance(dataset, temporaryMean, temporaryVariance);
    if (temporaryVariance <= 0)
      temporaryVariance = juce::jmax(1.0, std::abs(temporaryMean * 0.3));

    mean = temporaryMean;
    stddev = std::sqrt(temporaryVariance) >= std::pow(10.0, -5) ? std::sqrt(temporaryVariance)
        : std::pow(10.0, -5);
  }
#endif 

protected:
  friend class GaussianSamplerClass;

  double mean;
  double stddev;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CONTINUOUS_GAUSSIAN_H_
