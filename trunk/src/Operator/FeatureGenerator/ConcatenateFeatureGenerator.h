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

class ConcatenateFeatureGenerator : public FeatureGenerator
{
public:
  virtual EnumerationPtr getFeaturesEnumeration(ExecutionContext& context, TypePtr& elementsType)
  {
    size_t numInputs = getNumInputs();
    DefaultEnumerationPtr elementsEnumeration = new DefaultEnumeration(T("ConcatenatedFeatures"));
    elementsType = TypePtr();
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = getInputVariable(i);

      EnumerationPtr subElementsEnumeration;
      TypePtr subElementsType;
      if (!getDoubleVectorParameters(context, inputVariable->getType(), subElementsEnumeration, subElementsType))
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

    return new VariableSignature(computeOutputType(context) , T("AllFeatures"));
  }

  struct Callback : public VariableGeneratorCallback
  {
    Callback(VariableGeneratorCallback& target)
      : target(target) {}
    size_t shift;

    virtual void sense(size_t index, double value)
      {target.sense(index + shift, value);}

    VariableGeneratorCallback& target;
  };

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
  {
  /*  for (size_t i = 0; i < inputTypes.size(); ++i)
    {
      SparseDoubleObjectPtr input = inputs[i].dynamicCast<SparseDoubleObject>();
      jassert(input);
      res->appendValuesWithShift(input, shifts[i]);
    }*/
    jassert(false); 
    // FIXME: "LazyDoubleVector"
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    CompositeDoubleVectorPtr res = new CompositeDoubleVector(featuresEnumeration, featuresType);
    for (size_t i = 0; i < shifts.size(); ++i)
      res->appendSubVector(shifts[i], inputs[i].dynamicCast<DoubleVector>());
    return res;
  }

private:
  std::vector<size_t> shifts;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_
