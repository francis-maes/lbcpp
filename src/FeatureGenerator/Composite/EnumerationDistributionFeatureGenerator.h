/*-----------------------------------------.---------------------------------.
| Filename: EnumerationDistributionFeatu..h| Enumeration Discrete Distribution|
| Author  : Francis Maes                   |  Features                       |
| Started : 01/03/2011 16:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_COMPOSITE_ENUMERATION_DISTRIBUTION_H_
# define LBCPP_FEATURE_GENERATOR_COMPOSITE_ENUMERATION_DISTRIBUTION_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

// DV[enumeration, probabilityType] -> features
class EnumerationDistributionFeatureGenerator : public CompositeFunction
{
public:
  EnumerationDistributionFeatureGenerator(size_t probabilityDiscretization = 1, size_t entropyDiscretization = 10, double minEntropy = -1.0, double maxEntropy = 4.0)
    : probabilityDiscretization(probabilityDiscretization), entropyDiscretization(entropyDiscretization), minEntropy(minEntropy), maxEntropy(maxEntropy) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(doubleVectorClass(enumValueType, probabilityType), T("p"));

    EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(builder.getProvidedInput(0)->getType());
    if (!enumeration)
      return;
    
    std::vector<size_t> features;
    if (probabilityDiscretization)
    {
      if (probabilityDiscretization == 1)
        features.push_back(input);
      else
        for (size_t i = 0; i < enumeration->getNumElements(); ++i)
        {
          size_t position = builder.addConstant(i);
          size_t probability = builder.addFunction(getElementFunction(), input, position);
          EnumerationElementPtr element = enumeration->getElement(i);
          features.push_back(builder.addFunction(defaultProbabilityFeatureGenerator(probabilityDiscretization), probability, T("p[") + element->toShortString() + T("]")));
        }
    }

    if (entropyDiscretization)
    {
      size_t entropy = builder.addFunction(doubleVectorEntropyFunction(), input);
      features.push_back(builder.addFunction(defaultPositiveDoubleFeatureGenerator(entropyDiscretization, minEntropy, maxEntropy), entropy, T("e")));
    }

    if (features.size() > 1)
      builder.addFunction(concatenateFeatureGenerator(true), features);
  }

protected:
  friend class EnumerationDistributionFeatureGeneratorClass;

  size_t probabilityDiscretization;
  size_t entropyDiscretization;
  double minEntropy;
  double maxEntropy;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_COMPOSITE_ENUMERATION_DISTRIBUTION_H_
