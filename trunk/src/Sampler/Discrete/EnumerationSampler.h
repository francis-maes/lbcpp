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
      enumeration->getNumElements(), 1.0 / enumeration->getNumElements())), enumeration(enumeration) {}
  EnumerationSampler(const DenseDoubleVectorPtr& probabilities)
    : probabilities(probabilities), enumeration(probabilities->getElementsEnumeration()) {}
  EnumerationSampler() {}

  virtual String toShortString() const
  {
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
    {return Variable(random->sampleWithProbabilities(probabilities->getValues()), enumeration);}

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

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!DiscreteSampler::loadFromXml(importer))
      return false;
    enumeration = probabilities->getElementsEnumeration();
    return true;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    DiscreteSampler::clone(context, target);
    target.staticCast<EnumerationSampler>()->enumeration = enumeration;
  }

  virtual DenseDoubleVectorPtr computeLogProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    DenseDoubleVectorPtr returnProbabilities = new DenseDoubleVector(samples->getNumElements(), 0.0);

    for (size_t i = 0; i < samples->getNumElements(); i++)
    {
      size_t index = samples->getElement(i).getInteger();
      returnProbabilities->setValue(i, std::log(probabilities->getValue(index)));
    }

    return returnProbabilities;
  }

  protected:
  friend class EnumerationSamplerClass;

  DenseDoubleVectorPtr probabilities;
  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_ENUMERATION_DISCRETE_SAMPLER_H_
