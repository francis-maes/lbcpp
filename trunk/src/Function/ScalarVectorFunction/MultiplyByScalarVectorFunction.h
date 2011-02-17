/*-----------------------------------------.---------------------------------.
| Filename: MultiplyByScalarVectorFunction.h| ObjectFunction * scalar        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 14:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_MULTIPLY_BY_SCALAR_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_MULTIPLY_BY_SCALAR_H_

# include <lbcpp/Function/ScalarVectorFunction.h>

namespace lbcpp
{

class MultiplyByScalarVectorFunction : public ScalarVectorFunction
{
public:
  MultiplyByScalarVectorFunction(ScalarVectorFunctionPtr function, double scalar)
    : function(function), scalar(scalar) {}
  MultiplyByScalarVectorFunction() : scalar(0.0) {}

  virtual String toString() const
    {return T("(") + function->toString() + T(" x ") + String(scalar) + T(")");}

  virtual bool isDerivable() const
    {return function->isDerivable();}
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    if (scalar == 0)
      return;
    double out = 0.0;
    function->computeScalarVectorFunction(input, otherInputs, output ? &out : NULL, gradientTarget, gradientWeight * scalar);
    if (output)
      *output += out * scalar;
  }

protected:
  friend class MultiplyByScalarVectorFunctionClass;

  ScalarVectorFunctionPtr function;
  double scalar;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_MULTIPLY_BY_SCALAR_H_
