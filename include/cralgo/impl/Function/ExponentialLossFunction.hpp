/*-----------------------------------------.---------------------------------.
| Filename: ExponentialLossFuntion.hpp     | Exponential loss                |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_EXPONENTIAL_LOSS_H_
# define CRALGO_IMPL_FUNCTION_EXPONENTIAL_LOSS_H_

# include "LossFunctions.hpp"

namespace cralgo {
namespace impl {

// f(x) = exp(-x)
// f(x) > 0
struct ExponentialLossFunction : public ScalarFunction<ExponentialLossFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    double e = std::exp(-input);
    if (isNumberNearlyNull(e))
    {
      if (output)
        *output = 0;
      if (derivative)
        *derivative = 0;
    }
    else
    {
      if (derivative)
        *derivative = -e; 
      if (output)
        *output = e;
    }
  }
};
inline ExponentialLossFunction exponentialLossFunction()
  {return ExponentialLossFunction();}

STATIC_DISCRIMINANT_LOSS_FUNCTION(exponentialLoss, ExponentialLoss, ExponentialLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_EXPONENTIAL_LOSS_H_
