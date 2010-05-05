/*-----------------------------------------.---------------------------------.
| Filename: ScalarArchitectureExampleLos..h| Parameters -> Example Loss      |
| Author  : Francis Maes                   |  function                       |
| Started : 03/05/2010 15:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_SCALAR_ARCHITECTURE_EXAMPLE_LOSS_H_
# define LBCPP_CONTINUOUS_FUNCTION_SCALAR_ARCHITECTURE_EXAMPLE_LOSS_H_

# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

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
    jassert(isNumberValid(architectureOutput));
    double lossDerivative;
    lossFunction->compute(architectureOutput, output, NULL, parametersGradient ? &lossDerivative : NULL);
    if (parametersGradient)
    {
      jassert(isNumberValid(lossDerivative));
      *parametersGradient = multiplyByScalar(architectureOutputGradientWrtParameters, lossDerivative);
    }
    if (output)
      jassert(isNumberValid(*output));
  }

private:
  ScalarArchitecturePtr architecture;
  FeatureGeneratorPtr input;
  ScalarFunctionPtr lossFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_SCALAR_ARCHITECTURE_EXAMPLE_LOSS_H_
