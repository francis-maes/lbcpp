/*-----------------------------------------.---------------------------------.
| Filename: SmoothEnumerationSampler.h     | SmoothEnumerationSampler        |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 20 mai 2011  15:27:35          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_SMOOTH_ENUMERATION_SAMPLER_H_
# define LBCPP_SAMPLER_SMOOTH_ENUMERATION_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

namespace lbcpp
{

class SmoothEnumerationSampler : public DiscreteSampler
{
public:
  SmoothEnumerationSampler(EnumerationPtr enumeration, double learningRate, double probabilityForUnseenSample)
    : probabilities(new DenseDoubleVector(enumeration, probabilityType, enumeration->getNumElements(),
      1.0 / enumeration->getNumElements())), learningRate(learningRate),
      probabilityForUnseenSample(probabilityForUnseenSample), minValue(0), maxValue(enumeration->getNumElements()) {}
  SmoothEnumerationSampler(const DenseDoubleVectorPtr& probabilities, double learningRate, double probabilityForUnseenSample)
    : probabilities(probabilities), learningRate(learningRate),
      probabilityForUnseenSample(probabilityForUnseenSample), minValue(0), maxValue(probabilities->getNumValues() - 1) {}
  SmoothEnumerationSampler(const DenseDoubleVectorPtr& probabilities, double learningRate, double probabilityForUnseenSample, int minValue, int maxValue)
      : probabilities(probabilities), learningRate(learningRate),
        probabilityForUnseenSample(probabilityForUnseenSample), minValue(minValue), maxValue(maxValue) {}
  SmoothEnumerationSampler()
    : learningRate(0.1), probabilityForUnseenSample(0.1), minValue(0), maxValue(0) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t index = random->sampleWithProbabilities(probabilities->getValues());
    return juce::jlimit(minValue, maxValue, (int)index);
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    size_t n = trainingSamples->getNumElements();
    jassert(n);

    // Compute empirical frequencies
    DenseDoubleVectorPtr empiricalFrequencies = new DenseDoubleVector(probabilities->getNumValues(), 0.0);
    double increment = 1.0 / (double)n;
    for (size_t i = 0; i < n; i++)
    {
      size_t index = juce::jlimit(minValue, maxValue, trainingSamples->getElement(i).getInteger());
      empiricalFrequencies->incrementValue(index, increment);
    }

    // Update frequencies
    double difference = 0;
    double probabilityIncrement = 0;
    for (size_t j = 0; j < probabilities->getNumElements(); j++)
    {
      difference = empiricalFrequencies->getValue(j) - probabilities->getValue(j);
      probabilityIncrement = learningRate * difference;
      probabilities->setValue(j, juce::jmax(probabilityForUnseenSample, probabilities->getValue(j)
          + probabilityIncrement));
    }
  }

protected:
  friend class SmoothEnumerationSamplerClass;

  DenseDoubleVectorPtr probabilities;
  double learningRate;
  double probabilityForUnseenSample;
  int minValue;
  int maxValue;
};

typedef ReferenceCountedObjectPtr<SmoothEnumerationSampler> SmoothEnumerationSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_SMOOTH_ENUMERATION_SAMPLER_H_
