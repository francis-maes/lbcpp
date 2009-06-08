/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.cpp         | Continuous Functions            |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ContinuousFunction.h>
#include <lbcpp/impl/impl.h>
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

class VectorFunctionLineScalarFunction
  : public impl::StaticToDynamicScalarFunction< impl::VectorLineScalarFunction< impl::DynamicToStaticScalarVectorFunction > >
{
public:
  typedef impl::StaticToDynamicScalarFunction< impl::VectorLineScalarFunction< impl::DynamicToStaticScalarVectorFunction > > BaseClass;
  
  VectorFunctionLineScalarFunction(ScalarVectorFunctionPtr function, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
    : BaseClass(impl::vectorLineScalarFunction(impl::dynamicToStatic(function), parameters, direction)) {}
};

ScalarFunctionPtr ScalarVectorFunction::lineFunction(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction) const
{
  return new VectorFunctionLineScalarFunction(const_cast<ScalarVectorFunction* >(this), parameters, direction);
}

ScalarVectorFunctionPtr lbcpp::sumOfSquaresFunction(double weight)
{
  return weight != 1.0
    ? impl::staticToDynamic(impl::multiply(impl::sumOfSquares(), impl::constant(weight)))
    : impl::staticToDynamic(impl::sumOfSquares());
}
