/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorContinuousFun...hpp| Continuous functions f: R^d -> R|
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_CONTINUOUS_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_CONTINUOUS_H_

# include "ContinuousFunction.hpp"
# include "FunctionPairTraits.hpp"

namespace cralgo
{
namespace impl
{

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
    : architecture(architecture), input(input), weight(weight) {}

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

// f : params -> R
// f(theta) = 1/N sum_{i=1}^{N} (Architecture(theta) o Loss(y_i))(x_i)
template<class ExactType, class PenalizationType, class ContainerType>
struct EmpiricalRisk : public ScalarVectorFunction< ExactType >
{
  EmpiricalRisk(const PenalizationType& penalization, const ContainerType& examples)
    : penalization(penalization), examples(examples) {}

  PenalizationType penalization;
  const ContainerType&  examples;
  
  enum {isDerivable = PenalizationType::isDerivable};
  
  void compute(const FeatureGeneratorPtr parameters, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
  {
    DenseVectorPtr denseParameters = parameters.dynamicCast<DenseVector>();
    assert(denseParameters);
    
    if (output)
      *output = 0;
    if (!examples.size())
      return;
    double Z = 1.0 / examples.size();
    
    LazyVectorPtr lossGradient;
    if (gradient)
      lossGradient = new LazyVector();

    typedef Traits<ContainerType> ContainerTraits;
    for (typename ContainerTraits::ConstIterator it = ContainerTraits::begin(examples); it != ContainerTraits::end(examples); ++it)
    {
      typedef typename ContainerTraits::ValueType ValueType;
      const ValueType& example = ContainerTraits::value(it);

      const_cast<PenalizationType& >(penalization).right.setLearningExample(example);

      // FIXME : gradient direction
      if (lossGradient)
        lossGradient->clear();
      double lossOutput;
      penalization.compute(denseParameters, example.getInput(), output ? &lossOutput : NULL, lossGradient, LazyVectorPtr());
      double kZ = example.getWeight() * Z;
      if (output)
        *output += lossOutput * kZ;
      if (gradient)
        gradient->addWeighted(lossGradient, kZ);
    }
  }
};

/*
** Empirical risk with scalar outputs
*/
template<class ArchitectureType, class LossType, class ContainerType>
struct ScalarEmpiricalRisk : public EmpiricalRisk< 
      ScalarEmpiricalRisk<ArchitectureType, LossType, ContainerType> , 
      typename ScalarArchitectureScalarFunctionPair<ArchitectureType, LossType, void>::Composition,
      ContainerType
    >
{
  typedef ScalarEmpiricalRisk<ArchitectureType, LossType, ContainerType> ExactType;
  typedef typename ScalarArchitectureScalarFunctionPair<ArchitectureType, LossType, void>::Composition PenalizationType;
  typedef EmpiricalRisk<ExactType, PenalizationType, ContainerType> BaseClassType;
  
  ScalarEmpiricalRisk(const ArchitectureType& architecture, const LossType& loss, const ContainerType& examples) 
    : BaseClassType(PenalizationType(architecture, loss), examples) {}
};

template<class ArchitectureType, class LossType, class ContainerType>
inline ScalarEmpiricalRisk<ArchitectureType, LossType, ContainerType>
  empiricalRisk(const ScalarArchitecture<ArchitectureType>& architecture, const ScalarLossFunction<LossType>& loss, const ContainerType& examples)
  {return ScalarEmpiricalRisk<ArchitectureType, LossType, ContainerType >(static_cast<const ArchitectureType& >(architecture), static_cast<const LossType& >(loss), examples);}

/*
** Empirical risk with vector outputs
*/
template<class ArchitectureType, class LossType, class ContainerType>
struct VectorEmpiricalRisk : public EmpiricalRisk< 
      VectorEmpiricalRisk<ArchitectureType, LossType, ContainerType> , 
      typename VectorArchitectureScalarVectorFunctionPair<ArchitectureType, LossType, void>::Composition,
      ContainerType
    >
{
  typedef VectorEmpiricalRisk<ArchitectureType, LossType, ContainerType> ExactType;
  typedef typename VectorArchitectureScalarVectorFunctionPair<ArchitectureType, LossType, void>::Composition PenalizationType;
  typedef EmpiricalRisk<ExactType, PenalizationType, ContainerType> BaseClassType;
  
  VectorEmpiricalRisk(const ArchitectureType& architecture, const LossType& loss, const ContainerType& examples) 
    : BaseClassType(PenalizationType(architecture, loss), examples) {}
};

template<class ArchitectureType, class LossType, class ContainerType>
inline VectorEmpiricalRisk<ArchitectureType, LossType, ContainerType>
  empiricalRisk(const VectorArchitecture<ArchitectureType>& architecture, const VectorLossFunction<LossType>& loss, const ContainerType& examples)
  {return VectorEmpiricalRisk<ArchitectureType, LossType, ContainerType >(static_cast<const ArchitectureType& >(architecture), static_cast<const LossType& >(loss), examples);}


// f(x) = sum_i abs(x_i)
struct L1NormVectorContinuousFunction : public ScalarVectorFunction< L1NormVectorContinuousFunction >
{
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
  {
    if (output)
      *output = input->l1norm();
    if (gradient)
    {
      // FIXME: gradient = signs(input, gradientDirection)
    }
  }
};

// todo: loss de rankings

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_CONTINUOUS_H_
