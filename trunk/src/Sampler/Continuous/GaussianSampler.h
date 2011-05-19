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

class GaussianSampler : public ScalarContinuousSampler
{
public:
  GaussianSampler(double mean = 0.0, double stddev = 1.0)
    : mean(mean), stddev(stddev)
    {}

  virtual Variable computeExpectation(const Variable* inputs = NULL) const
    {return mean;}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleDoubleFromGaussian(mean, stddev);}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    if (trainingWeights)
    {
      jassert(trainingSamples->getNumElements() == trainingWeights->getNumElements());
      double denominator = 0;
      double tempMean = 0;
      double tempStdDev = 0;
      double gamma = 0;
      double value = 0;
      for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
      {
        gamma = trainingWeights->getValue(i);
        value = trainingSamples->getElement(i).getDouble();
        denominator += gamma;
        tempMean += gamma * value;
        tempStdDev += gamma * (value - mean) * (value - mean);
      }
      mean = tempMean / denominator;
      stddev = std::sqrt(tempStdDev / denominator);
    }
    else
    {
      // TODO: particular case when trainingSamples are DenseDoubleVectors
      size_t n = trainingSamples->getNumElements();
      ScalarVariableMeanAndVariance v;
      for (size_t i = 0; i < n; i++)
        v.push(trainingSamples->getElement(i).getDouble());
      mean = v.getMean();
      stddev = v.getStandardDeviation();
    }
  }

  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    double invDenom = 1.0 / (std::sqrt(2 * M_PI) * stddev);
    DenseDoubleVectorPtr result = new DenseDoubleVector(samples->getNumElements(), 0);
    for (size_t i = 0; i < samples->getNumElements(); i++)
    {
      double currentProbability = invDenom * std::exp(-0.5 * std::pow(
          (samples->getElement(i).getDouble() - mean) / stddev, 2.0));
      result->setValue(i, currentProbability);
    }
    return result;
  }

protected:
  friend class GaussianSamplerClass;

  double mean;
  double stddev;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CONTINUOUS_GAUSSIAN_H_
