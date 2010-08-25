/*-----------------------------------------.---------------------------------.
| Filename: AngleDifferenceFunction.h      | Compute the difference between  |
| Author  : Francis Maes                   |  angles                         |
| Started : 25/08/2010 18:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_ANGLE_DIFFERENCE_H_
# define LBCPP_FUNCTION_SCALAR_ANGLE_DIFFERENCE_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

class AngleDifferenceScalarFunction : public ScalarFunction
{
public:
  AngleDifferenceScalarFunction(double referenceAngle = 0.0)
    : referenceAngle(referenceAngle) {}

  virtual bool isDerivable() const
    {return false;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = normalizeAngle(input - referenceAngle);
    if (derivative)
      *derivative = 1.0;
  }

protected:
  friend class AngleDifferenceScalarFunctionClass;

  double referenceAngle;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_ANGLE_DIFFERENCE_H_
