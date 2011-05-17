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

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples)
  {
    // TODO: particular case when trainingSamples are DenseDoubleVectors
    size_t n = trainingSamples->getNumElements();
    ScalarVariableMeanAndVariance v;
    for (size_t i = 0; i < n; i++)
      v.push(trainingSamples->getElement(i).getDouble());
    mean = v.getMean();
    stddev = v.getStandardDeviation();
  }

  virtual void computeProbabilities(const ContainerPtr& data, DoubleMatrixPtr& probabilities,
      size_t numColumnToFill) const
  {
    jassert((data->getNumElements() == probabilities->getNumRows()) && (numColumnToFill < probabilities->getNumColumns()));
    double invDenom = 1.0 / (std::sqrt(2 * M_PI) * stddev);
    for (size_t i = 0; i < data->getNumElements(); i++)
    {
      double currentProbability = invDenom * std::exp(-0.5 * std::pow(
          (data->getElement(i).getDouble() - mean) / stddev, 2.0));
      probabilities->setValue(i, numColumnToFill, currentProbability);
    }
  }

  virtual void updateParameters(const ContainerPtr& data,
      const DoubleMatrixPtr& probabilitiesForAllModels, size_t numColumn)
  {
    jassert(data->getNumElements() == probabilitiesForAllModels->getNumRows());
    double denominator = 0;
    double tempMean = 0;
    double tempStdDev = 0;
    for (size_t i = 0; i < data->getNumElements(); i++)
    {
      denominator += probabilitiesForAllModels->getValue(i, numColumn);
      tempMean += probabilitiesForAllModels->getValue(i, numColumn)
          * data->getElement(i).getDouble();
      tempStdDev += probabilitiesForAllModels->getValue(i, numColumn) * std::pow(
          data->getElement(i).getDouble() - mean, 2.0);
    }
    mean = tempMean / denominator;
    stddev = std::sqrt(tempStdDev / denominator);
  }

protected:
  friend class GaussianSamplerClass;

  double mean;
  double stddev;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CONTINUOUS_GAUSSIAN_H_
