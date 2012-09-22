/*-----------------------------------------.---------------------------------.
| Filename: MultiplyByScalarFunction.h     | Multiply by scalar ScalarFuntion|
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_MULTIPLY_BY_SCALAR_H_
# define LBCPP_FUNCTION_SCALAR_MULTIPLY_BY_SCALAR_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

#if 0 
class MultiplyByScalarFunction : public ScalarFunction
{
public:
  MultiplyByScalarFunction(ScalarFunctionPtr function, double scalar)
    : function(function), scalar(scalar) {}
  MultiplyByScalarFunction() : scalar(0.0) {}

  virtual bool isDerivable() const
    {return function->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (scalar == 0.0)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (scalar == 1.0)
      function->compute(input, output, derivativeDirection, derivative);
    else
    {
      double dd;
      if (derivativeDirection)
        dd = *derivativeDirection * (scalar < 0.0 ? -1.0 : 1.0);
      function->compute(input, output, derivativeDirection ? &dd : NULL, derivative);
      if (output)
        *output *= scalar;
      if (derivative)
        *derivative *= scalar;
    }
  }

protected:
  friend class MultiplyByScalarFunctionClass;

  ScalarFunctionPtr function;
  double scalar;
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_MULTIPLY_BY_SCALAR_H_
