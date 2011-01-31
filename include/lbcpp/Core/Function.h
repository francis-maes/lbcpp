/*-----------------------------------------.---------------------------------.
| Filename: Function.h                     | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_H_
# define LBCPP_CORE_FUNCTION_H_

# include "Variable.h"

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

  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs, size_t numInputs) const
    {return checkNumInputsEquals(context, numInputs, 1) ? computeFunction(context, inputs[0]) : Variable();}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computeFunction(context, &input, 1);}

  virtual String getDescription(const Variable& input) const
    {return getClassName() + T("(") + input.toShortString() + T(")");}

  // push into stack
  void setPushIntoStackFlag(bool value)
    {pushIntoStack = value;}

  bool hasPushIntoStackFlag() const
    {return pushIntoStack;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionClass;

  bool pushIntoStack;

  bool checkNumInputsEquals(ExecutionContext& context, size_t numInputs, size_t requestedNumInputs) const;
  bool checkType(ExecutionContext& context, const Variable& variable, TypePtr type) const;
  bool checkExistence(ExecutionContext& context, const Variable& variable) const;
};

extern ClassPtr functionClass;

extern FunctionPtr identityFunction(TypePtr type);
extern FunctionPtr composeFunction(const FunctionPtr& f, const FunctionPtr& g);
extern FunctionPtr multiplyDoubleFunction();

extern FunctionPtr loadFromFileFunction(TypePtr expectedType = objectClass); // File -> Object
extern FunctionPtr loadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass);

extern FunctionPtr setFieldFunction(size_t fieldIndex); // (Object,Any) Pair -> Object
extern FunctionPtr selectVariableFunction(int index);
extern FunctionPtr selectPairVariablesFunction(int index1 = -1, int index2 = -1, TypePtr inputPairClass = pairClass(anyType, anyType));

extern FunctionPtr accumulateFunction(TypePtr inputType);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_H_
