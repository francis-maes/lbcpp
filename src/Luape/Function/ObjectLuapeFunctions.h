/*-----------------------------------------.---------------------------------.
| Filename: ObjectLuapeFunctions.h         | Object Luape Functions          |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_OBJECT_H_
# define LBCPP_LUAPE_FUNCTION_OBJECT_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>

namespace lbcpp
{

class GetVariableLuapeFunction : public LuapeFunction
{
public:
  GetVariableLuapeFunction(size_t variableIndex = 0)
    : variableIndex(variableIndex) {}

  virtual String toShortString() const
    {return ".var[" + String((int)variableIndex) + "]";}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(objectClass);}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
  {
    jassert(inputTypes.size() == 1);
    return inputTypes[0]->getMemberVariableType(variableIndex);
  }

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
  {
    jassert(inputs.size() == 1);
    VariableSignaturePtr member = inputs[0]->getType()->getMemberVariable(variableIndex);
    return inputs[0]->toShortString() + "." + (member->getShortName().isNotEmpty() ? member->getShortName() : member->getName());
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& input = inputs[0].getObject();
    if (input)
      return input->getVariable(variableIndex);
    else
      return Variable::missingValue(inputs[0].getType()->getMemberVariableType(variableIndex));
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    TypePtr objectClass = inputTypes[0];
    size_t n = objectClass->getNumMemberVariables();
    VectorPtr res = vector(positiveIntegerType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, i);
    return res;
  }

protected:
  friend class GetVariableLuapeFunctionClass;
  size_t variableIndex;
};

class GetContainerLengthLuapeFunction : public LuapeFunction
{
public:
  virtual String toShortString() const
    {return "length(.)";}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(containerClass());}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
    {return positiveIntegerType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {jassert(inputs.size() == 1); return "length(" + inputs[0]->toShortString() + ")";}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& input = inputs[0].getObjectAndCast<Container>();
    if (input)
      return Variable(input->getNumElements(), positiveIntegerType);
    else
      return Variable::missingValue(positiveIntegerType);
  }
};

class GetDoubleVectorElementLuapeFunction : public LuapeFunction
{
public:
  GetDoubleVectorElementLuapeFunction(size_t index = 0)
    : index(index) {}

  virtual String toShortString() const
    {return "[" + String((int)index) + "]";}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(type);
    return features && features->getNumElements() > 0;
  }

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
  {
    EnumerationPtr features;
    TypePtr elementsType;
    DoubleVector::getTemplateParameters(defaultExecutionContext(), inputTypes[0], features, elementsType);
    return elementsType;
  }

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
  {
    jassert(inputs.size() == 1);
    EnumerationPtr features = DoubleVector::getElementsEnumeration(inputs[0]->getType());
    jassert(features);
    return inputs[0]->toShortString() + "." + features->getElementName(index);
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();
    if (!input)
      return Variable::missingValue(doubleType);
    else
    {
      DenseDoubleVectorPtr denseInput = input.dynamicCast<DenseDoubleVector>();
      if (denseInput)
        return denseInput->getValue(index);
      else
        return input->getElement(index);
    }
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(inputTypes[0]);
    if (!features || features->getNumElements() == 0)
      return ContainerPtr();

    size_t n = features->getNumElements();
    VectorPtr res = vector(positiveIntegerType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, i);
    return res;
  }

protected:
  friend class GetDoubleVectorElementLuapeFunctionClass;
  size_t index;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_OBJECT_H_
