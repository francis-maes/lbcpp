/*-----------------------------------------.---------------------------------.
| Filename: GetVariableFunction.h          | Get Variable Function           |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2011 15:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_GET_VARIABLE_H_
# define LBCPP_CORE_FUNCTION_GET_VARIABLE_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class GetVariableFunction : public Function
{
public:
  GetVariableFunction(const String& variableName)
    : variableIndex((size_t)-1), variableName(variableName) {}
  GetVariableFunction(size_t variableIndex = 0)
    : variableIndex(variableIndex) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return objectClass;}

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
    return memberVariable->getType();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.getObject()->getVariable(variableIndex);}

protected:
  friend class GetVariableFunctionClass;

  size_t variableIndex;
  String variableName;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_GET_VARIABLE_H_
