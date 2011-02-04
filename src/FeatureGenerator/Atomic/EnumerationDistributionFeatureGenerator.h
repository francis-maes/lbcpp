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
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr distributionElementsType;
    if (!checkNumInputs(context, 1) || !checkInputType(context, 0, distributionClass(anyType)) || !Distribution::getTemplateParameter(context, getInputType(0), distributionElementsType))
      return EnumerationPtr();
    enumeration = distributionElementsType.dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return EnumerationPtr();
    }

    elementsType = probabilityType;
    outputName = inputVariables[0]->getName() + T("Features");
    outputShortName = inputVariables[0]->getShortName() + T("f");
    return addMissingToEnumerationEnumeration(enumeration);
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    EnumerationDistributionPtr distribution = inputs[0].getObjectAndCast<EnumerationDistribution>();
    const std::vector<double>& probabilities = distribution->getProbabilities();
    for (size_t i = 0; i < probabilities.size(); ++i)
      callback.sense(i, probabilities[i]);
  }

protected:
  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_DISTRIBUTION_H_
