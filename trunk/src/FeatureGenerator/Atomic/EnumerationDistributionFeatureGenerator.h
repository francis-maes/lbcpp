/*-----------------------------------------.---------------------------------.
| Filename: EnumerationDistr..eGenerator.h | EnumerationDistribution Features|
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 15:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_DISTRIBUTION_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_DISTRIBUTION_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

class EnumerationDistributionFeatureGenerator : public FeatureGenerator
{
public:
  EnumerationDistributionFeatureGenerator(bool includeMissingValue = true)
    : includeMissingValue(includeMissingValue) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return distributionClass(anyType);}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr distributionElementsType = Distribution::getTemplateParameter(inputVariables[0]->getType());
    enumeration = distributionElementsType.dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return EnumerationPtr();
    }

    elementsType = probabilityType;
    return includeMissingValue ? addMissingToEnumerationEnumeration(enumeration) : enumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    EnumerationDistributionPtr distribution = inputs[0].getObjectAndCast<EnumerationDistribution>();
    const std::vector<double>& probabilities = distribution->getProbabilities();
    size_t n = probabilities.size();
    if (!includeMissingValue)
      --n;
    for (size_t i = 0; i < probabilities.size(); ++i)
      callback.sense(i, probabilities[i]);
  }

protected:
  friend class EnumerationDistributionFeatureGeneratorClass;
  bool includeMissingValue;

  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_DISTRIBUTION_H_
