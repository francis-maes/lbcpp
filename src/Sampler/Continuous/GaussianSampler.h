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

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples)
  {
    // todo: particular case when trainingSamples are DenseDoubleVectors
    size_t n = trainingSamples->getNumElements();
    ScalarVariableMeanAndVariance v;
    for (size_t i = 0; i < n; i++)
      v.push(trainingSamples->getElement(i).getDouble());
    mean = v.getMean();
    stddev = v.getStandardDeviation();
  }

protected:
  friend class GaussianSamplerClass;

  double mean;
  double stddev;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CONTINUOUS_GAUSSIAN_H_
