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
  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  /**
  ** Computes the function
  **
  ** @param input : the input variable, whose type must inherit from getInputType()
  **
  ** @return a variable of type getOutputType(inputType)
  */
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const = 0;

  virtual String getDescription(const Variable& input) const
    {return getClassName() + T("(") + input.toShortString() + T(")");}

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr functionClass;

extern FunctionPtr identityFunction(TypePtr type);
extern FunctionPtr multiplyDoubleFunction();

extern FunctionPtr loadFromFileFunction(TypePtr expectedType = objectClass); // File -> Object
extern FunctionPtr loadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass);

extern FunctionPtr setFieldFunction(size_t fieldIndex); // (Object,Any) Pair -> Object
extern FunctionPtr selectVariableFunction(int index);
extern FunctionPtr selectPairVariablesFunction(int index1 = -1, int index2 = -1, TypePtr inputPairClass = pairClass(anyType, anyType));

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_H_
