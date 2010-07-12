/*-----------------------------------------.---------------------------------.
| Filename: Function.h                     | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VARIABLE_FUNCTION_H_
# define LBCPP_VARIABLE_FUNCTION_H_

# include "Variable.h"
# include "../ObjectPredeclarations.h"

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
  virtual TypePtr getInputType() const = 0;
  virtual TypePtr getOutputType(TypePtr inputType) const = 0;

  Variable compute(const Variable& input, ErrorHandler& callback = ErrorHandler::getInstance()) const
    {return checkInheritance(input, getInputType(), callback) ? computeFunction(input, callback) : Variable();}

protected:
  /**
  ** Computes the function
  **
  ** @param input : the input variable, whose type must inherit from getInputType()
  **
  ** @return a variable of type getOutputType(inputType)
  */
  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const = 0;
};

FunctionPtr loadFromFileFunction(); // File -> Variable

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_FUNCTION_H_
