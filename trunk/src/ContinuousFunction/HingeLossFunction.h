/*-----------------------------------------.---------------------------------.
| Filename: HingeLossFunction.h            | Hinge Loss Function             |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_HINGE_LOSS_H_
# define LBCPP_CONTINUOUS_FUNCTION_HINGE_LOSS_H_

# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

// f(x) = max(0, 1 - input)
class HingeLossFunction : public BinaryClassificationLossFunction
{
public:
  // correctClass: 0 = negative, 1 = positive
  HingeLossFunction(bool isPositive, double margin)
    : BinaryClassificationLossFunction(isPositive), margin(margin) {}
  HingeLossFunction() : margin(0.0) {}

  virtual bool isDerivable() const
    {return false;}

  virtual void computePositive(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (input > margin)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (input == margin)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = (!derivativeDirection || *derivativeDirection <= 0) ? -1.0 : 0;
    }
    else
    {
      if (output) *output = margin - input;
      if (derivative) *derivative = -1.0;
    }
  }

protected:
  double margin;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_HINGE_LOSS_H_
