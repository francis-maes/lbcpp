/*-----------------------------------------.---------------------------------.
| Filename: HingeLossFunction.hpp          | Hinge Loss Function             |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_HINGE_LOSS_H_
# define CRALGO_IMPL_FUNCTION_HINGE_LOSS_H_

# include "LossFunctions.hpp"

namespace cralgo {
namespace impl {

// f(x) = max(0, 1 - input)
struct HingeLossFunction : public ScalarFunction<HingeLossFunction>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (input >= 1)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (input == 1)
    {
      if (output) *output = 0.0;
      if (derivative) {assert(derivativeDirection); *derivative = derivativeDirection <= 0 ? -1 : 0;}
    }
    else
    {
      if (output) *output = 1 - input;
      if (derivative) *derivative = -1;
    }
  }
};
inline HingeLossFunction hingeLossFunction()
  {return HingeLossFunction();}

STATIC_DISCRIMINANT_LOSS_FUNCTION(hingeLoss, HingeLoss, HingeLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_HINGE_LOSS_H_
