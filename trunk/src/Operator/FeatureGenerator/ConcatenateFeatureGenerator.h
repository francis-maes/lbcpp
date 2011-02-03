/*-----------------------------------------.---------------------------------.
| Filename: ConcatenateFeatureGenerator.h  | Concatenate Feature Generator   |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_

# include <lbcpp/Operator/FeatureGenerator.h>

namespace lbcpp
{

class ConcatenateDoubleVectorFunction : public Function
{
public:
  ConcatenateDoubleVectorFunction(bool lazy = true)
    : lazy(lazy) {}

  EnumerationPtr getFeaturesEnumeration(ExecutionContext& context, TypePtr& elementsType)
  {
    size_t numInputs = getNumInputs();
    DefaultEnumerationPtr elementsEnumeration = new DefaultEnumeration(T("ConcatenatedFeatures"));
    elementsType = TypePtr();
    shifts.resize(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = getInputVariable(i);

      shifts[i] = elementsEnumeration->getNumElements();
      EnumerationPtr subElementsEnumeration;
      TypePtr subElementsType;
      if (!DoubleVector::getTemplateParameters(context, inputVariable->getType(), subElementsEnumeration, subElementsType))
        return EnumerationPtr();
      elementsEnumeration->addElementsWithPrefix(context, subElementsEnumeration, inputVariable->getName() + T("."), inputVariable->getShortName() + T("."));
      if (i == 0)
        elementsType = subElementsType;
      else
        elementsType = Type::findCommonBaseType(elementsType, subElementsType);
    }
    if (!elementsType->inheritsFrom(doubleType))
    {
      context.errorCallback(T("All elements do not inherit from double"));
      return EnumerationPtr();
    }
    return elementsEnumeration;
  }

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    size_t numInputs = getNumInputs();

    if (!numInputs)
    {
      context.errorCallback(T("No inputs"));
      return VariableSignaturePtr();
    }
    for (size_t i = 0; i < numInputs; ++i)
      if (!checkInputType(context, i, doubleVectorClass(enumValueType)))
        return VariableSignaturePtr();

    TypePtr featuresType = doubleType;
    EnumerationPtr featuresEnumeration = getFeaturesEnumeration(context, featuresType);
    TypePtr outputType = lazy
      ? compositeDoubleVectorClass(featuresEnumeration, featuresType)
      : sparseDoubleVectorClass(featuresEnumeration, featuresType);
    return new VariableSignature(outputType, T("AllFeatures"));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (lazy)
    {
      CompositeDoubleVectorPtr res = new CompositeDoubleVector(getOutputType());
      for (size_t i = 0; i < shifts.size(); ++i)
        res->appendSubVector(shifts[i], inputs[i].getObjectAndCast<DoubleVector>());
      return res;
    }
    else
    {
      SparseDoubleVectorPtr res = new SparseDoubleVector(getOutputType());
      for (size_t i = 0; i < shifts.size(); ++i)
        inputs[i].getObjectAndCast<DoubleVector>()->appendTo(res, shifts[i]);
      return res;
    }
  }

private:
  friend class ConcatenateDoubleVectorsFunctionClass;

  bool lazy;
  std::vector<size_t> shifts;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_
