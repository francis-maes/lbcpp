/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedRegressor.h       | Gradient based regressor        |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 14:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_REGRESSOR_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_REGRESSOR_H_

# include "StaticToDynamicGradientBasedMachine.h"
# include <lbcpp/Object/ObjectPair.h>

namespace lbcpp
{

class LeastSquaresLinearRegressor
  : public StaticToDynamicGradientBasedLearningMachine<LeastSquaresLinearRegressor, GradientBasedRegressor>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::SquareLoss<RegressionExample> loss() const
    {return impl::squareLoss<RegressionExample>();}
};

class GeneralizedLinearRegressor : public GradientBasedRegressor
{
public: 
  GeneralizedLinearRegressor() : architecture(linearArchitecture()) {}

  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return architecture;}

  static void getExample(ObjectPtr example, FeatureGeneratorPtr& input, ScalarFunctionPtr& lossFunction)
  {
    ObjectPairPtr inputLossPair = example.dynamicCast<ObjectPair>();
    jassert(inputLossPair);
    input = inputLossPair->getFirst().dynamicCast<FeatureGenerator>();
    jassert(input);
    lossFunction = inputLossPair->getSecond().dynamicCast<ScalarFunction>();
    jassert(lossFunction);
  }

  virtual FeatureDictionaryPtr getInputDictionaryFromExample(ObjectPtr example)
  {
    FeatureGeneratorPtr input;
    ScalarFunctionPtr lossFunction;
    getExample(example, input, lossFunction);
    return input->getDictionary();
  }

  virtual ScalarVectorFunctionPtr getLoss(ObjectPtr example) const
  {
    FeatureGeneratorPtr input;
    ScalarFunctionPtr lossFunction;
    getExample(example, input, lossFunction);
    return architecture->makeExampleLoss(input, lossFunction);
  }

  virtual ScalarVectorFunctionPtr getEmpiricalRisk(ObjectContainerPtr examples) const
    {return architecture->makeEmpiricalRisk(examples);}

  virtual ScalarVectorFunctionPtr getRegularizedEmpiricalRisk(ObjectContainerPtr examples) const
    {return sum(getEmpiricalRisk(examples), regularizer);}

private:
  ScalarArchitecturePtr architecture;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_REGRESSOR_H_
