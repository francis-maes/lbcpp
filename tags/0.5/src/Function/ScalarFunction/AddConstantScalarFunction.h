/*-----------------------------------------.---------------------------------.
| Filename: AddConstantScalarFunction.h    | Add Constant Scalar Function    |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_ADD_CONSTANT_H_
# define LBCPP_FUNCTION_SCALAR_ADD_CONSTANT_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{
#if 0
class AddConstantScalarFunction : public ScalarFunction
{
public:
  AddConstantScalarFunction(double constant)
    : constant(constant) {}
  AddConstantScalarFunction() {}

  virtual bool isDerivable() const
    {return true;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = input + constant;
    if (derivative)
      *derivative = 1.0;
  }

private:
  friend class AddConstantScalarFunctionClass;

  double constant;
};

class ScalarFunctionPlusConstant : public ScalarFunction
{
public:
  ScalarFunctionPlusConstant(ScalarFunctionPtr function, double constant)
    : function(function), constant(constant) {}
  ScalarFunctionPlusConstant() {}

  virtual bool isDerivable() const
    {return function->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    function->compute(input, output, derivativeDirection, derivative);
    if (output)
      *output += constant;
  }

protected:
  friend class ScalarFunctionPlusConstantClass;

  ScalarFunctionPtr function;
  double constant;
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_ADD_CONSTANT_H_
