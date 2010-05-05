/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.cpp         | Continuous Functions            |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/ContinuousFunction.h>
#include <lbcpp/impl/impl.h>
#include <lbcpp/ObjectPair.h>

#include "ContinuousFunction/MiscScalarFunctions.h"
#include "ContinuousFunction/BinarySumScalarVectorFunction.h"
#include "ContinuousFunction/ScalarArchitectureExampleLossFunction.h"
#include "ContinuousFunction/ScalarArchitectureEmpiricalRiskFunction.h"
#include "ContinuousFunction/HingeLossScalarFunction.h"
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
  {return impl::staticToDynamic(impl::squareFunction());}

ScalarFunctionPtr lbcpp::sum(ScalarFunctionPtr function, double constant)
  {return new ScalarFunctionPlusConstant(function, constant);}

ScalarFunctionPtr lbcpp::angleDifferenceScalarFunction(double reference)
  {return new AngleDifferenceScalarFunction(reference);}

ScalarFunctionPtr lbcpp::hingeLoss(size_t correctClass, double margin)
  {return new HingeLossScalarFunction(correctClass, margin);}

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

class SumOfSquaresScalarVectorFunction
  : public impl::StaticToDynamicScalarVectorFunction<impl::SumOfSquaresScalarVectorFunction>
{
public:
  typedef impl::StaticToDynamicScalarVectorFunction<impl::SumOfSquaresScalarVectorFunction> BaseClass;
  
  SumOfSquaresScalarVectorFunction() 
    : BaseClass(impl::sumOfSquares()) {}
};

class WeightedSumOfSquaresScalarVectorFunction 
  : public impl::StaticToDynamicScalarVectorFunction<
      impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication
    >
{
public:
  typedef impl::StaticToDynamicScalarVectorFunction<
      impl::ScalarVectorFunctionScalarConstantPair<impl::SumOfSquaresScalarVectorFunction, void>::Multiplication
    > BaseClass;
    
  WeightedSumOfSquaresScalarVectorFunction(double weight = 1.0)
    : BaseClass(BaseClass::ImplementationType(impl::sumOfSquares(), weight)) {}
};

ScalarVectorFunctionPtr lbcpp::sumOfSquaresFunction(double weight)
{
  if (weight == 1)
    return new SumOfSquaresScalarVectorFunction();
  return new WeightedSumOfSquaresScalarVectorFunction(weight);
    
//  return weight != 1.0 ? 
//    ? impl::staticToDynamic(impl::multiply(impl::sumOfSquares(), impl::constant(weight)))
//    : impl::staticToDynamic(impl::sumOfSquares());
}

ScalarVectorFunctionPtr lbcpp::sum(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2)
{
  if (f1)
    return f2 ? new BinarySumScalarVectorFunction(f1, f2) : f1;
  else
    return f2 ? f2 : ScalarVectorFunctionPtr();
}

/*
** ScalarArchitecture
*/
ScalarArchitecturePtr lbcpp::linearArchitecture()
  {return impl::staticToDynamic(impl::linearArchitecture());}

ScalarVectorFunctionPtr ScalarArchitecture::makeExampleLoss(FeatureGeneratorPtr input, ScalarFunctionPtr lossFunction) const
  {return new ScalarArchitectureExampleLossVectorFunction(ScalarArchitecturePtr(const_cast<ScalarArchitecture* >(this)), input, lossFunction);}

ScalarVectorFunctionPtr ScalarArchitecture::makeEmpiricalRisk(ObjectContainerPtr examples) const
  {return new ScalarArchitectureEmpiricalRiskVectorFunction(ScalarArchitecturePtr(const_cast<ScalarArchitecture* >(this)), examples);}

/*
** Serializable classes declaration
*/
void declareContinuousFunctions()
{
  LBCPP_DECLARE_CLASS(SumOfSquaresScalarVectorFunction);
  LBCPP_DECLARE_CLASS(WeightedSumOfSquaresScalarVectorFunction);
  LBCPP_DECLARE_CLASS(AngleDifferenceScalarFunction);
}
