/*-----------------------------------------.---------------------------------.
| Filename: AbsoluteLossFunction.hpp       | Absolute regression loss        |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 13:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_ABSOLUTE_LOSS_H_
# define LBCPP_CORE_IMPL_FUNCTION_ABSOLUTE_LOSS_H_

# include "LossFunctions.hpp"

namespace lbcpp {
namespace impl {

// f(x) = abs(x)
struct AbsoluteScalarFunction : public ScalarFunction<AbsoluteScalarFunction>
{
  enum {isDerivable = false};

  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
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
      {
        assert(derivativeDirection);
        if (*derivativeDirection > 0)
          *derivative = 1;
        else if (*derivativeDirection < 0)
          *derivative = -1;
        else
          *derivative = 0;
      }
    }
  }
};
inline AbsoluteScalarFunction absFunction()
  {return AbsoluteScalarFunction();}

STATIC_REGRESSION_LOSS_FUNCTION(absoluteLoss, AbsoluteLoss, AbsoluteScalarFunction);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_ABSOLUTE_LOSS_H_
