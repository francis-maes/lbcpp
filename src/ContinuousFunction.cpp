/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.cpp         | Continuous Functions            |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ContinuousFunction.h>
#include <lbcpp/impl/impl.h>
#include <lbcpp/ObjectPair.h>
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

class ScalarFunctionComposition : public ScalarFunction
{
public:
  ScalarFunctionComposition(ScalarFunctionPtr f1, ScalarFunctionPtr f2)
    : f1(f1), f2(f2) {}

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
  ScalarFunctionPtr f1;
  ScalarFunctionPtr f2;
};

ScalarFunctionPtr ScalarFunction::composeWith(ScalarFunctionPtr postFunction) const
  {return new ScalarFunctionComposition(ScalarFunctionPtr(const_cast<ScalarFunction* >(this)), postFunction);}

class AddConstantScalarFunction : public ScalarFunction
{
public:
  AddConstantScalarFunction(double constant)
    : constant(constant) {}

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
  double constant;
};

ScalarFunctionPtr lbcpp::addConstantScalarFunction(double constant)
  {return new AddConstantScalarFunction(constant);}

ScalarFunctionPtr lbcpp::squareFunction()
  {return impl::staticToDynamic(impl::squareFunction());}

class ScalarFunctionPlusConstant : public ScalarFunction
{
public:
  ScalarFunctionPlusConstant(ScalarFunctionPtr function, double constant)
    : function(function), constant(constant) {}

  virtual bool isDerivable() const
    {return function->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    function->compute(input, output, derivativeDirection, derivative);
    if (output)
      *output += constant;
  }

protected:
  ScalarFunctionPtr function;
  double constant;
};

ScalarFunctionPtr lbcpp::sum(ScalarFunctionPtr function, double constant)
  {return new ScalarFunctionPlusConstant(function, constant);}

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

class ScalarVectorFunctionBinarySum : public ScalarVectorFunction
{
public:
  ScalarVectorFunctionBinarySum(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2)
    : f1(f1), f2(f2) {}

  virtual bool isDerivable() const
    {return f1->isDerivable() && f2->isDerivable();}

  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    double output1, output2;
    FeatureGeneratorPtr gradient1, gradient2;
    f1->compute(input, output ? &output1 : NULL, gradientDirection, gradient ? &gradient1 : NULL);
    f2->compute(input, output ? &output2 : NULL, gradientDirection, gradient ? &gradient2 : NULL);
    if (output)
      *output = output1 + output2;
    if (gradient)
      *gradient = weightedSum(gradient1, 1.0, gradient2, 1.0);
  }

protected:
  ScalarVectorFunctionPtr f1;
  ScalarVectorFunctionPtr f2;
};

ScalarVectorFunctionPtr lbcpp::sum(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2)
{
  if (f1)
    return f2 ? new ScalarVectorFunctionBinarySum(f1, f2) : f1;
  else
    return f2 ? f2 : ScalarVectorFunctionPtr();
}

/*
** ScalarArchitecture
*/
ScalarArchitecturePtr lbcpp::linearArchitecture()
  {return impl::staticToDynamic(impl::linearArchitecture());}

class ScalarArchitectureExampleLossVectorFunction : public ScalarVectorFunction
{
public:
  ScalarArchitectureExampleLossVectorFunction(ScalarArchitecturePtr architecture, FeatureGeneratorPtr input, ScalarFunctionPtr lossFunction)
    : architecture(architecture), input(input), lossFunction(lossFunction) {}

  virtual bool isDerivable() const
    {return lossFunction->isDerivable() && architecture->isDerivable();}

  virtual void compute(const FeatureGeneratorPtr parameters, double* output, const FeatureGeneratorPtr parametersGradientDirection, FeatureGeneratorPtr* parametersGradient) const
    {compute(architecture, input, lossFunction, parameters, output, parametersGradientDirection, parametersGradient);}

  static void compute(ScalarArchitecturePtr architecture, FeatureGeneratorPtr input, ScalarFunctionPtr lossFunction, 
    const FeatureGeneratorPtr parameters, double* output, const FeatureGeneratorPtr parametersGradientDirection, FeatureGeneratorPtr* parametersGradient)
  {
    double architectureOutput;
    FeatureGeneratorPtr architectureOutputGradientWrtParameters;
    architecture->compute(parameters, input, &architectureOutput, parametersGradient ? &architectureOutputGradientWrtParameters : NULL, NULL);
    double lossDerivative;
    lossFunction->compute(architectureOutput, output, NULL, parametersGradient ? &lossDerivative : NULL);
    if (parametersGradient)
      *parametersGradient = multiplyByScalar(architectureOutputGradientWrtParameters, lossDerivative);
  }

private:
  ScalarArchitecturePtr architecture;
  FeatureGeneratorPtr input;
  ScalarFunctionPtr lossFunction;
};

ScalarVectorFunctionPtr ScalarArchitecture::makeExampleLoss(FeatureGeneratorPtr input, ScalarFunctionPtr lossFunction) const
  {return new ScalarArchitectureExampleLossVectorFunction(ScalarArchitecturePtr(const_cast<ScalarArchitecture* >(this)), input, lossFunction);}

class ScalarArchitectureEmpiricalRiskVectorFunction : public ScalarVectorFunction
{
public:
  ScalarArchitectureEmpiricalRiskVectorFunction(ScalarArchitecturePtr architecture, ObjectContainerPtr examples)
    : architecture(architecture), examples(examples) {}

  virtual bool isDerivable() const
    {return architecture->isDerivable();}

  virtual void compute(const FeatureGeneratorPtr parameters, double* output, const FeatureGeneratorPtr parametersGradientDirection, FeatureGeneratorPtr* parametersGradient) const
  {
    if (output)
      *output = 0;
    if (!examples->size())
    {
      if (parametersGradient)
        *parametersGradient = emptyFeatureGenerator();
      return;
    }
    double invZ = 1.0 / examples->size();
    
    std::vector<std::pair<FeatureGeneratorPtr, double> >* gradientLinearCombination = NULL;
    if (parametersGradient)
    {
      gradientLinearCombination = new std::vector<std::pair<FeatureGeneratorPtr, double> >();
      gradientLinearCombination->reserve(examples->size());
    }

    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->getAndCast<ObjectPair>(i);
      jassert(example);
      FeatureGeneratorPtr input = example->getFirst().dynamicCast<FeatureGenerator>();
      ScalarFunctionPtr lossFunction = example->getFirst().dynamicCast<ScalarFunction>();

      // FIXME : gradient direction
      double lossOutput;
      FeatureGeneratorPtr lossGradient;
      ScalarArchitectureExampleLossVectorFunction::compute(architecture, input, lossFunction,
        parameters, output ? &lossOutput : NULL, FeatureGeneratorPtr(), parametersGradient ? &lossGradient : NULL);
      if (output)
        *output += lossOutput * invZ;
      if (parametersGradient)
        gradientLinearCombination->push_back(std::make_pair(lossGradient, invZ));
    }
    
    if (parametersGradient)
      *parametersGradient = linearCombination(gradientLinearCombination);
  }

private:
  ScalarArchitecturePtr architecture;
  ObjectContainerPtr examples;
};

ScalarVectorFunctionPtr ScalarArchitecture::makeEmpiricalRisk(ObjectContainerPtr examples) const
  {return new ScalarArchitectureEmpiricalRiskVectorFunction(ScalarArchitecturePtr(const_cast<ScalarArchitecture* >(this)), examples);}

/*
** Serializable classes declaration
*/
void declareContinuousFunctions()
{
  LBCPP_DECLARE_CLASS(SumOfSquaresScalarVectorFunction);
  LBCPP_DECLARE_CLASS(WeightedSumOfSquaresScalarVectorFunction);
}
