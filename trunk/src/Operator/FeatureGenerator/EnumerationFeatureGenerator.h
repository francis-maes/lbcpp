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
  virtual EnumerationPtr getFeaturesEnumeration(ExecutionContext& context, TypePtr& elementsType)
  {
    elementsType = probabilityType;
    return addMissingToEnumerationEnumeration(enumeration);
  }

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    enumeration = getInputType(0).dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return VariableSignaturePtr();
    }
    return new VariableSignature(computeOutputType(context), inputVariables[0]->getName() + T("Features"), inputVariables[0]->getShortName() + T("f"));
  }

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
    {callback.sense(getIndex(*inputs), 1.0);}

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
