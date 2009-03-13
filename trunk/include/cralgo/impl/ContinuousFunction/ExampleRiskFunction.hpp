/*-----------------------------------------.---------------------------------.
| Filename: ExampleRiskFunction.hpp        | f : parameters -> loss of a     |
| Author  : Francis Maes                   |   given example                 |
| Started : 11/03/2009 19:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_EXAMPLE_RISK_H_
# define CRALGO_IMPL_FUNCTION_EXAMPLE_RISK_H_

# include "FunctionStatic.hpp"
# include "FunctionPairTraits.hpp"

namespace cralgo {
namespace impl {

/*
** Scalar Architecture => Function of the parameters
*/
// transform "features x params => scalar" into "params => scalar" for a given feature set
// f(x) = Architecture(input, x) * weight
template<class ArchitectureType>
struct ScalarArchitectureToParametersFunction 
  : public ScalarVectorFunction< ScalarArchitectureToParametersFunction<ArchitectureType> >
{
  ScalarArchitectureToParametersFunction(const ArchitectureType& architecture, const FeatureGeneratorPtr input, double weight = 1.0)
    : architecture(architecture), input(input), weight(weight) {assert(input);}

  ArchitectureType architecture;
  const FeatureGeneratorPtr input;
  double weight;
  
  enum {isDerivable = ArchitectureType::isDerivable};
  
  void compute(const FeatureGeneratorPtr parameters, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
  {
    DenseVectorPtr denseParameters = parameters.dynamicCast<DenseVector>();
    assert(denseParameters);
    // FIXME: use gradientDirection
    architecture.compute(denseParameters, input, output, gradient, LazyVectorPtr());
    if (output)
      *output *= weight;
    if (gradient)
      gradient->multiplyByScalar(weight);
  }
};

template<class ArchitectureType>
inline ScalarArchitectureToParametersFunction<ArchitectureType>
  architectureToParametersFunction(const ScalarArchitecture<ArchitectureType>& architecture, const FeatureGeneratorPtr input, double weight)
  {return ScalarArchitectureToParametersFunction<ArchitectureType>(static_cast<const ArchitectureType& >(architecture), input, weight);}
  
template<class ArchitectureType, class LossType, class ExampleType>
inline ScalarArchitectureToParametersFunction<
    typename ScalarArchitectureScalarFunctionPair<ArchitectureType, LossType, void>::Composition >
  exampleRisk(const ScalarArchitecture<ArchitectureType>& architecture, const ScalarLossFunction<LossType>& loss, const ExampleType& example)
{
  typedef typename ScalarArchitectureScalarFunctionPair<ArchitectureType, LossType, void>::Composition ComposedArchitecture;
  ComposedArchitecture arch = compose(static_cast<const ArchitectureType& >(architecture), static_cast<const LossType& >(loss));
  arch.right.setLearningExample(example);
  return architectureToParametersFunction(arch, example.getInput(), example.getWeight());
}

template<class ArchitectureType, class LossType, class ExampleType>
inline ScalarArchitectureToParametersFunction<
    typename VectorArchitectureScalarVectorFunctionPair<ArchitectureType, LossType, void>::Composition >
  exampleRisk(const VectorArchitecture<ArchitectureType>& architecture, const VectorLossFunction<LossType>& loss, const ExampleType& example)
{
  typedef typename VectorArchitectureScalarVectorFunctionPair<ArchitectureType, LossType, void>::Composition ComposedArchitecture;
  ComposedArchitecture arch = compose(static_cast<const ArchitectureType& >(architecture), static_cast<const LossType& >(loss));
  arch.right.setLearningExample(example);
  return architectureToParametersFunction(arch, example.getInput(), example.getWeight());
}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_EXAMPLE_RISK_H_
