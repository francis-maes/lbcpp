/*-----------------------------------------.---------------------------------.
| Filename: ScalarArchitectureEmpiricalR..h| Parameters -> Empirical Risk    |
| Author  : Francis Maes                   |  function                       |
| Started : 03/05/2010 15:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_SCALAR_ARCHITECTURE_EMPIRICAL_RISK_H_
# define LBCPP_CONTINUOUS_FUNCTION_SCALAR_ARCHITECTURE_EMPIRICAL_RISK_H_

# include <lbcpp/ContinuousFunction.h>

namespace lbcpp
{

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
      ScalarFunctionPtr lossFunction = example->getSecond().dynamicCast<ScalarFunction>();
      jassert(input && lossFunction);

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

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_SCALAR_ARCHITECTURE_EMPIRICAL_RISK_H_
