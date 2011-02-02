/*-----------------------------------------.---------------------------------.
| Filename: EnumerationFeatureGenerator.h  | Enumeration Features            |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 15:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_H_

# include <lbcpp/Operator/FeatureGenerator.h>

namespace lbcpp
{

class EnumerationFeatureGenerator : public FeatureGenerator
{
public:
  virtual TypePtr initializeOperator(ExecutionContext& context)
  {
    if (!checkNumInputsEquals(context, 1))
      return TypePtr();
    enumeration = getInputType(0).dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return TypePtr();
    }
    return enumBasedDoubleVectorClass(addMissingToEnumerationEnumeration(enumeration), probabilityType);
  }

  virtual double dotProduct(const Variable* inputs, size_t startIndex, double weight, const DenseDoubleObjectPtr& parameters) const
    {return parameters->getValue(startIndex + getIndex(*inputs)) * weight;}

  virtual void computeFeatures(const Variable* inputs, size_t startIndex, double weight, const SparseDoubleObjectPtr& target) const
    {target->appendValue(startIndex + getIndex(*inputs), weight);}

protected:
  EnumerationPtr enumeration;

  size_t getIndex(const Variable& input) const
  {
    int index = input.getInteger();
    jassert(index >= 0 && index <= (int)enumeration->getNumElements());
    return (size_t)index;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_ENUMERATION_H_
