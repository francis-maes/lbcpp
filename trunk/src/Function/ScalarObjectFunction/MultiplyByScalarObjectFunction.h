/*-----------------------------------------.---------------------------------.
| Filename: MultiplyByScalarObjectFunction.h| ObjectFunction * scalar        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 14:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_MULTIPLY_BY_SCALAR_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_MULTIPLY_BY_SCALAR_H_

# include <lbcpp/Function/ScalarObjectFunction.h>

namespace lbcpp
{

class MultiplyByScalarObjectFunction : public ScalarObjectFunction
{
public:
  MultiplyByScalarObjectFunction(ScalarObjectFunctionPtr function, double scalar)
    : function(function), scalar(scalar) {}
  MultiplyByScalarObjectFunction() : scalar(0.0) {}

  virtual String toString() const
    {return T("(") + function->toString() + T(" x ") + String(scalar) + T(")");}

  virtual bool isDerivable() const
    {return function->isDerivable();}
  
  virtual void compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
  {
    if (scalar == 0)
      return;
    double out = 0.0;
    function->compute(input, output ? &out : NULL, gradientTarget, gradientWeight * scalar);
    if (output)
      *output += out * scalar;
  }

protected:
  friend class MultiplyByScalarObjectFunctionClass;

  ScalarObjectFunctionPtr function;
  double scalar;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_MULTIPLY_BY_SCALAR_H_
