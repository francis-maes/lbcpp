/*-----------------------------------------.---------------------------------.
| Filename: MultiplyFunction.h             | Multiply Function               |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 14:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_MULTIPLY_H_
# define LBCPP_FUNCTION_MULTIPLY_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class MultiplyDoubleFunction : public Function
{
public:
  virtual TypePtr getInputType() const
    {return pairClass(doubleType, doubleType);}

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input[0].getDouble() * input[1].getDouble();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_MULTIPLY_H_
