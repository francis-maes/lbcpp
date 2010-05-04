/*-----------------------------------------.---------------------------------.
| Filename: HingeLossScalarFunction.h      | Hinge Loss Function             |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_HINGE_LOSS_H_
# define LBCPP_CONTINUOUS_FUNCTION_HINGE_LOSS_H_

# include <lbcpp/ContinuousFunction.h>

namespace lbcpp
{

// f(x) = max(0, 1 - input)
class HingeLossScalarFunction : public ScalarFunction
{
public:
  // correctClass: 0 = negative, 1 = positive
  HingeLossScalarFunction(size_t correctClass, double margin)
    : correctClass(correctClass), margin(margin) {jassert(correctClass <= 1);}
  HingeLossScalarFunction() : correctClass(0), margin(0.0) {}

  virtual String toString() const
    {return String(T("HingeLoss(")) + (correctClass ? T("+") : T("-")) + T(")");}

  virtual bool isDerivable() const
    {return false;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    double sign = correctClass ? 1.0 : -1.0;
    if (!correctClass)
      input = -input;

    if (input >= margin)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (input == margin)
    {
      if (output) *output = 0.0;
      if (derivative) {jassert(derivativeDirection); *derivative = derivativeDirection <= 0 ? -sign : 0;}
    }
    else
    {
      if (output) *output = 1 - input;
      if (derivative) *derivative = -sign;
    }
  }

protected:
  size_t correctClass;
  double margin;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_HINGE_LOSS_H_
