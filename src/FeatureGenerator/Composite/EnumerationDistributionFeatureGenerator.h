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
# include <lbcpp/Distribution/DiscreteDistribution.h>

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
    size_t entropy = builder.addFunction(doubleVectorEntropyFunction(), input);
    size_t entropyFeatures = builder.addFunction(defaultPositiveDoubleFeatureGenerator(entropyDiscretization, minEntropy, maxEntropy), entropy, T("e"));
    builder.addFunction(concatenateFeatureGenerator(false), input, entropyFeatures);
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
