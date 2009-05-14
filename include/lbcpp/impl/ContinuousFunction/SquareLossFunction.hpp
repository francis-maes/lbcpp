/*-----------------------------------------.---------------------------------.
| Filename: SquareLossFunction.hpp         | Square regression loss          |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 13:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_SQUARE_LOSS_H_
# define LBCPP_CORE_IMPL_FUNCTION_SQUARE_LOSS_H_

# include "LossFunctions.hpp"

namespace lbcpp {
namespace impl {

// f(x) = x^2
struct SquareScalarFunction : public ScalarFunction<SquareScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output)
      *output = input * input;
    if (derivative)
      *derivative = 2 * input;
  }
};
inline SquareScalarFunction squareFunction()
  {return SquareScalarFunction();}

STATIC_REGRESSION_LOSS_FUNCTION(squareLoss, SquareLoss, SquareScalarFunction);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_SQUARE_LOSS_H_
