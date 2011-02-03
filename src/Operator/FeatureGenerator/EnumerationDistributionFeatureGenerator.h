/*-----------------------------------------.---------------------------------.
| Filename: EnumerationDistr..eGenerator.h | EnumerationDistribution Features|
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 15:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_DISTRIBUTION_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_DISTRIBUTION_H_

# include <lbcpp/Operator/FeatureGenerator.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

class EnumerationDistributionFeatureGenerator : public FeatureGenerator
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1) || !checkInputType(context, 0, distributionClass(anyType)))
      return VariableSignaturePtr();
    TypePtr elementsType = getDistributionElementsType(context, getInputType(0));
    if (!elementsType)
      return VariableSignaturePtr();
    enumeration = elementsType.dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return VariableSignaturePtr();
    }
    TypePtr type = enumBasedDoubleVectorClass(addMissingToEnumerationEnumeration(enumeration), probabilityType);
    return new VariableSignature(type, inputVariables[0]->getName() + T("Features"), inputVariables[0]->getShortName() + T("f"));
  }

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
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

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_DISTRIBUTION_H_
