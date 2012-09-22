/*-----------------------------------------.---------------------------------.
| Filename: ClamperSampler.h               | Clamper Sampler                 |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 9, 2012  8:14:41 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_CLAMPERSAMPLER_H_
# define LBCPP_SAMPLER_CLAMPERSAMPLER_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class ClamperSampler : public CompositeSampler
{
public:
  ClamperSampler(double minValue, double maxValue, const ScalarContinuousSamplerPtr& sampler)
    : CompositeSampler(1), minValue(minValue), maxValue(maxValue), mean((minValue + maxValue) / 2)
    {samplers[0] = sampler;}
  ClamperSampler() : CompositeSampler(1), minValue(0), maxValue(0), mean(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    double value = samplers[0]->sample(context, random, inputs).getDouble();

    value *= 0.5 * (maxValue - minValue);
    value += mean;
    value = juce::jlimit(minValue, maxValue, value);
    return value;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
  {
    double normalize = 1.0 / (0.5 * (maxValue - minValue));
    VectorPtr samples = new DenseDoubleVector(trainingSamples->getNumElements(), 0.0);
    for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
    {
      double valueToSet = juce::jlimit(minValue, maxValue, trainingSamples->getElement(i).getDouble());
      valueToSet -= mean;
      valueToSet *= normalize;
      samples->setElement(i, valueToSet);
    }

    samplers[0]->learn(context, trainingInputs, samples, trainingWeights, validationInputs,
        validationSamples, supervisionWeights);
  }

  virtual DenseDoubleVectorPtr computeLogProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    double normalize = 1.0 / (0.5 * (maxValue - minValue));
    ContainerPtr newSamples = new DenseDoubleVector(samples->getNumElements(), 0.0);
    for (size_t i = 0; i < samples->getNumElements(); i++)
    {
      double valueToSet = juce::jlimit(minValue, maxValue, samples->getElement(i).getDouble());
      valueToSet -= mean;
      valueToSet *= normalize;
      newSamples->setElement(i, valueToSet);
    }

    return samplers[0]->computeLogProbabilities(inputs, newSamples);
  }

protected:
  friend class ClamperSamplerClass;

  double minValue;
  double maxValue;
  double mean;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CLAMPERSAMPLER_H_
