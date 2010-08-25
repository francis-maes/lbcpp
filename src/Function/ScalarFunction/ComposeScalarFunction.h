/*-----------------------------------------.---------------------------------.
| Filename: ComposeScalarFunction.h        | Scalar Function Composition     |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_COMPOSE_H_
# define LBCPP_FUNCTION_SCALAR_COMPOSE_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

class ComposeScalarFunction : public ScalarFunction
{
public:
  ComposeScalarFunction(ScalarFunctionPtr f1, ScalarFunctionPtr f2)
    : f1(f1), f2(f2) {}
  ComposeScalarFunction() {}

  virtual bool isDerivable() const
    {return f1->isDerivable() && f2->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    double f1output, f1derivative;
    f1->compute(input, &f1output, derivativeDirection, derivative ? &f1derivative : NULL);
    f2->compute(f1output, output, NULL, derivative);
    if (derivative)
      *derivative *= f1derivative;
  }

private:
  friend class ComposeScalarFunctionClass;

  ScalarFunctionPtr f1;
  ScalarFunctionPtr f2;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_COMPOSE_H_
