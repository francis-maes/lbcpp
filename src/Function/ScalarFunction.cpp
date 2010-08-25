/*-----------------------------------------.---------------------------------.
| Filename: ScalarFunction.cpp             | R -> R Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarFunction.h>
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
ScalarFunctionPtr lbcpp::addConstantScalarFunction(double constant)
  {return new AddConstantScalarFunction(constant);}

ScalarFunctionPtr lbcpp::squareFunction()
  {return new SquareFunction();}

ScalarFunctionPtr lbcpp::absFunction()
  {return new AbsFunction();}

ScalarFunctionPtr lbcpp::sum(ScalarFunctionPtr function, double constant)
  {return new ScalarFunctionPlusConstant(function, constant);}

ScalarFunctionPtr lbcpp::angleDifferenceScalarFunction(double reference)
  {return new AngleDifferenceScalarFunction(reference);}

BinaryClassificationLossFunctionPtr lbcpp::hingeLoss(bool isPositive, double margin)
  {return new HingeLossFunction(isPositive, margin);}

BinaryClassificationLossFunctionPtr lbcpp::logBinomialLoss(bool isPositive)
  {return new LogBinomialLossFunction(isPositive);}*/

/*
** BinaryClassificationLossFunction
*/
String BinaryClassificationLossFunction::toString() const
  {return getClassName() + T("(") + (isPositive ? T("+") : T("-")) + T(")");}

void BinaryClassificationLossFunction::compute(double input, double* output, const double* derivativeDirection, double* derivative) const
{
  double dd;
  if (derivativeDirection)
    dd = isPositive ? *derivativeDirection : -(*derivativeDirection);
  computePositive(isPositive ? input : -input, output, derivativeDirection ? &dd : NULL, derivative);
  if (derivative && !isPositive)
    *derivative = - (*derivative);
}

/*
void declareContinuousFunctions()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(ContinuousFunction, Object);
  LBCPP_DECLARE_ABSTRACT_CLASS(ScalarFunction, ContinuousFunction);

  LBCPP_DECLARE_CLASS(ScalarFunctionComposition, ScalarFunction);
  LBCPP_DECLARE_CLASS(MultiplyByScalarFunction, ScalarFunction);

  LBCPP_DECLARE_CLASS(AngleDifferenceScalarFunction, ScalarFunction);
  LBCPP_DECLARE_CLASS(AbsFunction, ScalarFunction);
  LBCPP_DECLARE_CLASS(SquareFunction, ScalarFunction);

  LBCPP_DECLARE_ABSTRACT_CLASS(BinaryClassificationLossFunction, ScalarFunction);
  LBCPP_DECLARE_CLASS(HingeLossFunction, BinaryClassificationLossFunction);
  LBCPP_DECLARE_CLASS(LogBinomialLossFunction, BinaryClassificationLossFunction);

  LBCPP_DECLARE_ABSTRACT_CLASS(ScalarVectorFunction, ContinuousFunction);
  LBCPP_DECLARE_CLASS(SumOfSquaresScalarVectorFunction, ScalarVectorFunction);
}
*/