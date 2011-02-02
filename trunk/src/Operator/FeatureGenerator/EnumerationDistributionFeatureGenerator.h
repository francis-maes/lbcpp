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
  virtual TypePtr initializeOperator(ExecutionContext& context)
  {
    if (!checkNumInputsEquals(context, 1) || !checkInputType(context, 0, distributionClass(anyType)))
      return TypePtr();
    TypePtr elementsType = getDistributionElementsType(context, inputTypes[0]);
    if (!elementsType)
      return TypePtr();
    enumeration = elementsType.dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return TypePtr();
    }
    return enumBasedDoubleVectorClass(addMissingToEnumerationEnumeration(enumeration), probabilityType);
  }

  virtual double dotProduct(const Variable* inputs, size_t startIndex, double weight, const DenseDoubleObjectPtr& parameters) const
  {
    EnumerationDistributionPtr distribution = inputs[0].getObjectAndCast<EnumerationDistribution>();
    const std::vector<double>& probabilities = distribution->getProbabilities();
    double res = 0.0;
    for (size_t i = 0; i < probabilities.size(); ++i)
      res += parameters->getValue(startIndex + i) * probabilities[i];
    return res * weight;
  }

  virtual void computeFeatures(const Variable* inputs, size_t startIndex, double weight, const SparseDoubleObjectPtr& target) const
  {
    EnumerationDistributionPtr distribution = inputs[0].getObjectAndCast<EnumerationDistribution>();
    const std::vector<double>& probabilities = distribution->getProbabilities();
    for (size_t i = 0; i < probabilities.size(); ++i)
      target->appendValue(startIndex++, probabilities[i]);
  }

protected:
  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_DISTRIBUTION_H_
