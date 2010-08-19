/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.cpp         | Continuous Functions            |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/ContinuousFunction.h>
#include "ContinuousFunction/MiscScalarFunctions.h"
#include "ContinuousFunction/BinarySumScalarVectorFunction.h"
#include "ContinuousFunction/HingeLossFunction.h"
#include "ContinuousFunction/LogBinomialLossFunction.h"
#include "ContinuousFunction/SquareFunction.h"
#include "ContinuousFunction/AbsFunction.h"
#include "ContinuousFunction/MultiplyByScalarVectorFunction.h"
#include "ContinuousFunction/SumOfSquaresScalarVectorFunction.h"
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

ObjectPtr ScalarFunction::multiplyByScalar(double scalar)
  {return new MultiplyByScalarFunction(ScalarFunctionPtr(const_cast<ScalarFunction* >(this)), scalar);}

ScalarFunctionPtr ScalarFunction::composeWith(ScalarFunctionPtr postFunction) const
  {return new ScalarFunctionComposition(ScalarFunctionPtr(const_cast<ScalarFunction* >(this)), postFunction);}

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
  {return new LogBinomialLossFunction(isPositive);}

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
** ScalarVectorFunction
*/
double ScalarVectorFunction::compute(const FeatureGeneratorPtr input) const
{
  double res;
  compute(input, &res, FeatureGeneratorPtr(), NULL);
  return res;
}

FeatureGeneratorPtr ScalarVectorFunction::computeGradient(const FeatureGeneratorPtr input) const
{
  FeatureGeneratorPtr res;
  compute(input, NULL, FeatureGeneratorPtr(), &res);
  return res;
}

FeatureGeneratorPtr ScalarVectorFunction::computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const
{
  FeatureGeneratorPtr res;
  compute(input, NULL, gradientDirection, &res);
  return res;
}

void ScalarVectorFunction::compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const
{
  compute(input, output, FeatureGeneratorPtr(), gradient);
}

bool ScalarVectorFunction::checkDerivativeWrtDirection(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
{
  double dirNorm = direction->l2norm();
  double epsilon = 5e-6 / dirNorm;
  double value1 = compute(weightedSum(parameters, 1.0, direction, -epsilon, true));
  double value2 = compute(weightedSum(parameters, 1.0, direction, epsilon, true));
  double numericalDerivative = (value2 - value1) / (2.0 * epsilon);
  FeatureGeneratorPtr gradient = computeGradient(parameters, direction);
  double analyticDerivative = gradient->dotProduct(direction);
  Object::warning("ScalarVectorFunction::checkDerivativeWrtDirection",
    "Derivative Check: " + lbcpp::toString(numericalDerivative) + " vs. " + lbcpp::toString(analyticDerivative));
  return fabs(numericalDerivative - analyticDerivative) < 0.00001;
}

ObjectPtr ScalarVectorFunction::multiplyByScalar(double scalar)
{
  ScalarVectorFunctionPtr pthis(const_cast<ScalarVectorFunction* >(this));
  return scalar == 1.0
    ? ObjectPtr(pthis)
    : ObjectPtr(new MultiplyByScalarVectorFunction(pthis, scalar));
}

ScalarVectorFunctionPtr lbcpp::sumOfSquaresFunction()
  {return new SumOfSquaresScalarVectorFunction();}

ScalarVectorFunctionPtr lbcpp::sum(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2)
{
  if (f1)
    return f2 ? new BinarySumScalarVectorFunction(f1, f2) : f1;
  else
    return f2 ? f2 : ScalarVectorFunctionPtr();
}

/*
** Serializable classes declaration
*/
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
