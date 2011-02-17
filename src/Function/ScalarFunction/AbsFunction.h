/*-----------------------------------------.---------------------------------.
| Filename: AbsFunction.h                  | Absolute value function         |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 13:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_ABSOLUTE_VALUE_H_
# define LBCPP_FUNCTION_SCALAR_ABSOLUTE_VALUE_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

// f(x) = abs(x)
class AbsFunction : public ScalarFunction
{
public:
  virtual bool isDerivable() const
    {return false;}

  virtual void computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const
  {
    if (output)
      *output = fabs(input);
    if (derivative)
    {
      if (input > 0)
        *derivative = 1;
      else if (input < 0)
        *derivative = -1;
      else
        *derivative = 0;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_ABSOLUTE_VALUE_H_
