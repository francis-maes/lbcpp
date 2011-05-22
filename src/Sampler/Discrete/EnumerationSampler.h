/*-----------------------------------------.---------------------------------.
| Filename: EnumerationSampler.h           | Enumeration Sampler             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  11:11:01         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_DISCRETE_ENUMERATION_H_
# define LBCPP_SAMPLER_DISCRETE_ENUMERATION_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class EnumerationSampler : public DiscreteSampler
{
public:
  EnumerationSampler(EnumerationPtr enumeration)
    : probabilities(new DenseDoubleVector(enumeration, probabilityType,
      enumeration->getNumElements(), 1.0 / enumeration->getNumElements())) {}
  EnumerationSampler(const DenseDoubleVectorPtr& probabilities)
    : probabilities(probabilities) {}
  EnumerationSampler() {}

  virtual String toShortString() const
  {
    EnumerationPtr enumeration = probabilities->getElementsEnumeration();
    std::set<size_t> nonNullProbabilities;
    size_t n = probabilities->getNumValues();
    for (size_t i = 0; i < n; ++i)
      if (probabilities->getValue(i) > 1e-12)
        nonNullProbabilities.insert(i);
    if (nonNullProbabilities.size() == 1)
      return enumeration->getElementName(*nonNullProbabilities.begin());
    else if (nonNullProbabilities.size() <= 4)
    {
      String res;
      for (std::set<size_t>::const_iterator it = nonNullProbabilities.begin(); it != nonNullProbabilities.end(); ++it)
      {
        if (res.isNotEmpty())
          res += T(", ");
        res += enumeration->getElementName(*it) + T(": ") + Variable(probabilities->getValue(*it), probabilityType).toShortString();
      }
      return res;
    }
    else
      return T("distribution over ") + enumeration->getName();
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleWithProbabilities(probabilities->getValues());}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    size_t n = trainingSamples->getNumElements();
    jassert(n);

    // Compute empirical frequencies
    DenseDoubleVectorPtr empiricalFrequencies = new DenseDoubleVector(probabilities->getClass());
    double increment = 1.0 / (double)n;
    for (size_t i = 0; i < n; i++)
    {
      size_t index = trainingSamples->getElement(i).getInteger();
      empiricalFrequencies->incrementValue(index, increment);
    }
    probabilities = empiricalFrequencies;
  }

protected:
  friend class EnumerationSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_
