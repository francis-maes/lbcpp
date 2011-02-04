/*-----------------------------------------.---------------------------------.
| Filename: SelectPairVariablesFunction.h  | Select Pair Fields Function     |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
# define LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

// NEW 
class GetVariableFunction : public Function
{
public:
  GetVariableFunction(const String& variableName)
    : variableIndex((size_t)-1), variableName(variableName) {}
  GetVariableFunction(size_t variableIndex = 0)
    : variableIndex(variableIndex) {}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1) && !checkInputType(context, 0, objectClass))
      return VariableSignaturePtr();
    const VariableSignaturePtr& inputVariable = getInputVariable(0);
    const TypePtr& inputType = inputVariable->getType();

    if (variableName.isNotEmpty())
    {
      int index = inputType->findMemberVariable(variableName);
      if (index < 0)
      {
        context.errorCallback(T("Variable ") + variableName + T(" does not exists in class ") + inputType->getName());
        return VariableSignaturePtr();
      }
      variableIndex = (size_t)index;
    }
    else if (variableIndex >= inputType->getNumMemberVariables())
    {
      context.errorCallback(String((int)variableIndex) + T(" is not a valid member variable index for class ") + inputType->getName());
      return VariableSignaturePtr();
    }

    const VariableSignaturePtr& memberVariable = inputType->getMemberVariable(variableIndex);
    return new VariableSignature(memberVariable->getType(), inputVariable->getName() + T(".") + memberVariable->getName(),
                                  memberVariable->getShortName(), memberVariable->getDescription());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.getObject()->getVariable(variableIndex);}

protected:
  friend class GetVariableFunctionClass;

  size_t variableIndex;
  String variableName;
};

// Container<T>, PositiveInteger -> T
class GetElementFunction : public Function
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    TypePtr elementsType;
    if (!checkNumInputs(context, 2) ||
        !Container::getTemplateParameter(context, getInputType(0), elementsType) ||
        !checkInputType(context, 1, positiveIntegerType))
      return VariableSignaturePtr();

    const VariableSignaturePtr& firstInputVariable = getInputVariable(0);
    return new VariableSignature(elementsType, firstInputVariable->getName() + T("Element"), firstInputVariable->getShortName() + T("[]"));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    int index = inputs[1].getInteger();
    if (index >= 0 && index < (int)container->getNumElements())
      return container->getElement((size_t)index);
    else
      return Variable::missingValue(container->getElementsType());
  }
};

// OLD - deprecated
// a => a.x
class SelectVariableFunction : public Function
{
public:
  SelectVariableFunction(int index = -1)
    : index(index) {}

  virtual TypePtr getInputType() const
    {return objectClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return index >= 0 ? inputType->getMemberVariableType((size_t)index) : inputType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (index >= 0)
    {
      Variable res = input.getObject()->getVariable(index);
      if (!res.exists())
        res = Variable::missingValue(input.getType()->getMemberVariableType((size_t)index));
      return res;
    }
    else
      return input;
  }

private:
  friend class SelectVariableFunctionClass;

  int index;
};

// pair(a, b) => pair(a.x, b.y)
class SelectPairVariablesFunction : public Function
{
public:
  SelectPairVariablesFunction(int index1, int index2, TypePtr inputPairClass)
    : index1(index1), index2(index2), inputPairClass(inputPairClass)
    {computeOutputType();}
  SelectPairVariablesFunction() : index1(-1), index2(-1) {}

  virtual TypePtr getInputType() const
    {return inputPairClass;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return outputType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    jassert(pair);
    const Variable& first = pair->getFirst();
    const Variable& second = pair->getSecond();
    return new Pair(outputType, index1 >= 0 ? first.getObject()->getVariable(index1) : first,
                                index2 >= 0 ? second.getObject()->getVariable(index2) : second);
  }

  virtual bool loadFromXml(XmlImporter& importer)
    {return Function::loadFromXml(importer) && computeOutputType();}

private:
  friend class SelectPairVariablesFunctionClass;

  int index1, index2;
  TypePtr inputPairClass;

  TypePtr outputType;

  bool computeOutputType()
  {
    outputType = pairClass(
      getOutputTypeBase(inputPairClass->getTemplateArgument(0), index1),
      getOutputTypeBase(inputPairClass->getTemplateArgument(1), index2));
    return true;
  }

  static TypePtr getOutputTypeBase(TypePtr inputType, int index)
  {
    if (index >= 0)
    {
      jassert((size_t)index < inputType->getNumMemberVariables());
      return inputType->getMemberVariableType((size_t)index);
    }
    else
      return inputType;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
