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

namespace lbcpp
{

class EnumerationDiscreteSampler;
typedef ReferenceCountedObjectPtr<EnumerationDiscreteSampler> EnumerationDiscreteSamplerPtr;

class EnumerationDiscreteSampler : public DiscreteSampler
{
public:
  EnumerationDiscreteSampler(EnumerationPtr enumeration)
  {
    probabilities = new DenseDoubleVector(enumeration, probabilityType);
  }

  EnumerationDiscreteSampler(const DenseDoubleVectorPtr& probabilities) : probabilities(probabilities) {}

  EnumerationDiscreteSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleWithNormalizedProbabilities(probabilities->getValues());}

  /**
   * dataset = first : a Variable of integer type that corresponds to the element observed.
   *           second : not yet used.
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    jassert(dataset.size());

    // Compute empirical frequencies
    DenseDoubleVectorPtr empiricalFrequencies = new DenseDoubleVector(
        probabilities->getElementsEnumeration(), probabilities->getElementsType(),
        probabilities->getNumElements(), 0.0);
    double increment = 1.0 / (double)dataset.size();
    for (size_t i = 0; i < dataset.size(); i++)
    {
      size_t index = dataset[i].getInteger();
      empiricalFrequencies->setValue(index, empiricalFrequencies->getValue(index) + increment);
    }
  }

protected:
  friend class EnumerationDiscreteSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_
