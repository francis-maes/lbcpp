/*-----------------------------------------.---------------------------------.
| Filename: SetVariableFunction.h          | Set Variable Function           |
| Author  : Julien Becker                  |                                 |
| Started : 11/07/2011 13:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_SET_VARIABLE_H_
# define LBCPP_CORE_FUNCTION_SET_VARIABLE_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class SetVariableFunction : public Function
{
public:
  SetVariableFunction(const String& variableName)
    : variableIndex((size_t)-1), variableName(variableName) {}
  SetVariableFunction(size_t variableIndex = 0)
    : variableIndex(variableIndex) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)objectClass;}

  virtual String getOutputPostFix() const
    {return T("Member");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    const VariableSignaturePtr& inputVariable = inputVariables[0];
    const TypePtr& inputType = inputVariable->getType();

    if (variableName.isNotEmpty())
    {
      int index = inputType->findMemberVariable(variableName);
      if (index < 0)
      {
        context.errorCallback(T("Variable ") + variableName + T(" does not exists in class ") + inputType->getName());
        return TypePtr();
      }
      variableIndex = (size_t)index;
    }
    else if (variableIndex >= inputType->getNumMemberVariables())
    {
      context.errorCallback(String((int)variableIndex) + T(" is not a valid member variable index for class ") + inputType->getName());
      return TypePtr();
    }
    
    const VariableSignaturePtr& memberVariable = inputType->getMemberVariable(variableIndex);
    outputName = inputVariable->getName() + T(".") + memberVariable->getName();
    outputShortName = memberVariable->getShortName();
    if (!context.checkInheritance(inputVariables[1]->getType(), memberVariable->getType()))
      return TypePtr();
    return inputType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr input = inputs[0].getObject();
    input->setVariable(variableIndex, inputs[1]);
    return input;
  }

protected:
  friend class SetVariableFunctionClass;

  size_t variableIndex;
  String variableName;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_SET_VARIABLE_H_
