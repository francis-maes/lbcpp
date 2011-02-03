/*-----------------------------------------.---------------------------------.
| Filename: ScalarFunction.cpp             | R -> R Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarFunction.h>
#include <lbcpp/Function/ScalarObjectFunction.h>
using namespace lbcpp;

/*
** Scalar Function
*/
double ScalarFunction::compute(double input) const
{
  double res;
  compute(input, &res, NULL, NULL);
  return res;
}

double ScalarFunction::computeDerivative(double input) const
{
  double res;
  compute(input, NULL, NULL, &res);
  return res;
}

double ScalarFunction::computeDerivative(double input, double direction) const
{
  double res;
  compute(input, NULL, &direction, &res);
  return res;
}

void ScalarFunction::compute(double input, double* output, double* derivative) const
  {compute(input, output, NULL, derivative);}

ScalarFunctionPtr ScalarFunction::multiplyByScalar(double scalar)
  {return multiplyByScalarFunction(refCountedPointerFromThis(this), scalar);}

ScalarFunctionPtr ScalarFunction::composeWith(ScalarFunctionPtr postFunction) const
  {return composeScalarFunction(refCountedPointerFromThis(this), postFunction);}

/*
** ScalarObjectFunction
*/
ScalarObjectFunctionPtr ScalarObjectFunction::multiplyByScalar(double weight) const
  {return multiplyByScalarObjectFunction(refCountedPointerFromThis(this), weight);}
