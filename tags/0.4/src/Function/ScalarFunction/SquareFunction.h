/*-----------------------------------------.---------------------------------.
| Filename: SquareFunction.h               | Square function                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 13:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SCALAR_FUNCTION_SQUARE_H_
# define LBCPP_SCALAR_FUNCTION_SQUARE_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

// f(x) = x^2
class SquareFunction : public ScalarFunction
{
public:
  virtual bool isDerivable() const
    {return true;}

  virtual void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output)
      *output = input * input;
    if (derivative)
      *derivative = 2 * input;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SCALAR_FUNCTION_SQUARE_H_
