/*-----------------------------------------.---------------------------------.
| Filename: EmpiricalRiskFunction.hpp      | Empirical risk given a set of   |
| Author  : Francis Maes                   |   examples. A function from     |
| Started : 07/03/2009 15:39               | params to                       |
`------------------------------------------/        1/n sum_i loss(x_i, y_i) |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_EMPIRICAL_RISK_H_
# define CRALGO_IMPL_FUNCTION_EMPIRICAL_RISK_H_

# include "FunctionStatic.hpp"
# include "FunctionPairTraits.hpp"

namespace cralgo
{
namespace impl
{

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


}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_EMPIRICAL_RISK_H_
