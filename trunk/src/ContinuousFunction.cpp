/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.cpp         | Continuous Functions            |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/ContinuousFunction.h>
#include <lbcpp/Object/ObjectPair.h>

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

BinaryClassificationLossFunctionPtr lbcpp::hingeLoss(size_t correctClass, double margin)
  {return new HingeLossFunction(correctClass, margin);}

BinaryClassificationLossFunctionPtr lbcpp::logBinomialLoss(size_t correctClass)
  {return new LogBinomialLossFunction(correctClass);}

/*
** BinaryClassificationLossFunction
*/
BinaryClassificationLossFunction::BinaryClassificationLossFunction(size_t correctClass)
  : correctClass(correctClass) {jassert(correctClass <= 1);}

String BinaryClassificationLossFunction::toString() const
  {return getClassName() + T("(") + (correctClass ? T("+") : T("-")) + T(")");}

void BinaryClassificationLossFunction::compute(double input, double* output, const double* derivativeDirection, double* derivative) const
{
  bool invertSign = (correctClass == 0);
  double dd;
  if (derivativeDirection)
    dd = invertSign ? -(*derivativeDirection) : *derivativeDirection;
  computePositive(invertSign ? -input : input, output, derivativeDirection ? &dd : NULL, derivative);
  if (derivative && invertSign)
    *derivative = - (*derivative);
}

bool BinaryClassificationLossFunction::load(InputStream& istr)
  {return ScalarFunction::load(istr) && lbcpp::read(istr, correctClass);}

void BinaryClassificationLossFunction::save(OutputStream& ostr) const
  {ScalarFunction::save(ostr); lbcpp::write(ostr, correctClass);}

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

ScalarVectorFunctionPtr ScalarVectorFunction::multiplyByScalar(double scalar) const
{
  ScalarVectorFunctionPtr pthis(const_cast<ScalarVectorFunction* >(this));
  return scalar == 1.0
    ? pthis
    : new MultiplyByScalarVectorFunction(pthis, scalar);
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
  LBCPP_DECLARE_CLASS(AngleDifferenceScalarFunction);
  LBCPP_DECLARE_CLASS(HingeLossFunction);
  LBCPP_DECLARE_CLASS(LogBinomialLossFunction);
  LBCPP_DECLARE_CLASS(SumOfSquaresScalarVectorFunction);
  LBCPP_DECLARE_CLASS(AbsFunction);
  LBCPP_DECLARE_CLASS(SquareFunction);
}
