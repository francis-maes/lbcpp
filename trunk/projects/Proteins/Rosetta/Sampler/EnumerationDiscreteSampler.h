/*-----------------------------------------.---------------------------------.
| Filename: EnumerationDiscreteSampler.h   | EnumerationDiscreteSampler      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  11:11:01         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

# define DEFAULT_PROBABILITY_FOR_UNSEEN_SAMPLES 0.001
# define LEARNING_RATE 0.1

namespace lbcpp
{

class EnumerationDiscreteSampler;
typedef ReferenceCountedObjectPtr<EnumerationDiscreteSampler> EnumerationDiscreteSamplerPtr;

class EnumerationDiscreteSampler : public DiscreteSampler
{
public:
  EnumerationDiscreteSampler()
    : DiscreteSampler(), probabilityForUnseenSamples(DEFAULT_PROBABILITY_FOR_UNSEEN_SAMPLES)
  {
  }

  EnumerationDiscreteSampler(size_t numElements, double probabilityForUnseenSamples =
      DEFAULT_PROBABILITY_FOR_UNSEEN_SAMPLES)
    : DiscreteSampler(), probabilityForUnseenSamples(probabilityForUnseenSamples)
  {
    probabilities = new DenseDoubleVector(denseDoubleVectorClass(
        positiveIntegerEnumerationEnumeration), numElements, 1.0 / (double)numElements);
  }

  EnumerationDiscreteSampler(DenseDoubleVectorPtr& probabilities,
      double probabilityForUnseenSamples = DEFAULT_PROBABILITY_FOR_UNSEEN_SAMPLES)
    : DiscreteSampler(), probabilityForUnseenSamples(probabilityForUnseenSamples)
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    this->probabilities = new DenseDoubleVector(actionClass, probabilities->getValues());
  }

  EnumerationDiscreteSampler(const EnumerationDiscreteSampler& sampler)
    : DiscreteSampler(), probabilityForUnseenSamples(sampler.probabilityForUnseenSamples)
  {
    probabilities = new DenseDoubleVector(denseDoubleVectorClass(
        positiveIntegerEnumerationEnumeration), sampler.probabilities->getValues());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    return Variable(random->sampleWithNormalizedProbabilities(probabilities->getValues()));
  }

  /**
   * dataset = first : a Variable of integer type that corresponds to the element observed.
   *           second : not yet used.
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if (dataset.size() < 2)
      return;

    // Compute empirical frequencies
    DenseDoubleVectorPtr empiricalFrequencies = new DenseDoubleVector(denseDoubleVectorClass(
        positiveIntegerEnumerationEnumeration), probabilities->getNumElements(), 0.0);
    double increment = 1.0 / (double)dataset.size();
    for (size_t i = 0; i < dataset.size(); i++)
    {
      size_t index = dataset[i].getInteger();
      empiricalFrequencies->setValue(index, empiricalFrequencies->getValue(index) + increment);
    }

    // Update frequencies
    double totalNorm = 0;
    for (size_t i = 0; i < probabilities->getNumElements(); i++)
    {
      probabilities->setValue(i, juce::jmax(probabilityForUnseenSamples, probabilities->getValue(i)
          + LEARNING_RATE * (empiricalFrequencies->getValue(i) - probabilities->getValue(i))));
      totalNorm += probabilities->getValue(i);
    }

    double normalize = 1.0 / totalNorm;
    for (size_t i = 0; i < probabilities->getNumElements(); i++)
      probabilities->setValue(i, probabilities->getValue(i) * normalize);
  }

protected:
  friend class EnumerationDiscreteSamplerClass;
  DenseDoubleVectorPtr probabilities;
  double probabilityForUnseenSamples;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_
