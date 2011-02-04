/*-----------------------------------------.---------------------------------.
| Filename: Function.h                     | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_FUNCTION_H_
# define LBCPP_FUNCTION_FUNCTION_H_

# include "../Core/Variable.h"
# include "../Core/Pair.h"
# include "../Core/Vector.h"
# include "predeclarations.h"

namespace lbcpp
{
  
/**
** @class Function
** @brief Represents a function which takes a Variable as input and
** returns a Variable.
**
** Function can be applied to streams and to containers.
**
** @see Stream, Container
*/
class Function : public Object
{
public:
  Function() : pushIntoStack(false) {}

  // new
  bool initialize(ExecutionContext& context, TypePtr inputType);
  bool initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables);

protected:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
    {jassert(false); return VariableSignaturePtr();}

public:
  size_t getNumInputs() const
    {return inputVariables.size();}

  const VariableSignaturePtr& getInputVariable(size_t index) const
    {jassert(index < inputVariables.size()); return inputVariables[index];}

  const TypePtr& getInputType(size_t index) const
    {return getInputVariable(index)->getType();}

  const VariableSignaturePtr& getOutputVariable() const
    {return outputVariable;}

  const TypePtr& getOutputType() const
    {return outputVariable->getType();}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassert(getNumInputs() == 1); return computeFunction(context, &input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {jassert(getNumInputs() == 1); return computeFunction(context, *inputs);}

  // old
  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  virtual String getDescription(const Variable& input) const
    {return getClassName() + T("(") + input.toShortString() + T(")");}
  // -

  // push into stack
  void setPushIntoStackFlag(bool value)
    {pushIntoStack = value;}

  bool hasPushIntoStackFlag() const
    {return pushIntoStack;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionClass;

  bool pushIntoStack;

  std::vector<VariableSignaturePtr> inputVariables;
  VariableSignaturePtr outputVariable;

  bool checkNumInputs(ExecutionContext& context, size_t numInputs) const;
  bool checkInputType(ExecutionContext& context, size_t index, TypePtr requestedType) const;
  
  bool checkExistence(ExecutionContext& context, const Variable& variable) const;
};

extern ClassPtr functionClass;

// new
extern FunctionPtr getVariableFunction(size_t variableIndex);
extern FunctionPtr getVariableFunction(const String& variableName);
extern FunctionPtr getElementFunction();

// old
extern FunctionPtr identityFunction(TypePtr type);
extern FunctionPtr composeFunction(const FunctionPtr& f, const FunctionPtr& g);
extern FunctionPtr multiplyDoubleFunction();

extern FunctionPtr loadFromFileFunction(TypePtr expectedType = objectClass); // File -> Object
extern FunctionPtr loadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass);

extern FunctionPtr setFieldFunction(size_t fieldIndex); // (Object,Any) Pair -> Object
extern FunctionPtr selectVariableFunction(int index);
extern FunctionPtr selectPairVariablesFunction(int index1 = -1, int index2 = -1, TypePtr inputPairClass = pairClass(anyType, anyType));
// -



class ProxyFunction : public Function
{
protected:
  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    implementation = createImplementation(inputVariables);
    if (!implementation)
    {
      context.errorCallback(T("Could not create implementation in proxy operator"));
      return VariableSignaturePtr();
    }
    if (!implementation->initialize(context, inputVariables))
      return VariableSignaturePtr();

    return implementation->getOutputVariable();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassert(implementation); return implementation->computeFunction(context, input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {jassert(implementation); return implementation->computeFunction(context, inputs);}

  FunctionPtr implementation;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_H_
