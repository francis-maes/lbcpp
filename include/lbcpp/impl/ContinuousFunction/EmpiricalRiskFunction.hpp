/*-----------------------------------------.---------------------------------.
| Filename: EmpiricalRiskFunction.hpp      | Empirical risk given a set of   |
| Author  : Francis Maes                   |   examples. A function from     |
| Started : 07/03/2009 15:39               | params to                       |
`------------------------------------------/        1/n sum_i loss(x_i, y_i) |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_EMPIRICAL_RISK_H_
# define LBCPP_CORE_IMPL_FUNCTION_EMPIRICAL_RISK_H_

# include "FunctionStatic.hpp"
# include "FunctionPairTraits.hpp"
# include "../../ObjectContainer.h"

namespace lbcpp {
namespace impl {

// f : params -> R
// f(theta) = 1/N sum_{i=1}^{N} (Architecture(theta) o Loss(y_i))(x_i)
template<class ExactType, class PenalizationType, class ExampleType>
struct EmpiricalRisk : public ScalarVectorFunction< ExactType >
{
  typedef ReferenceCountedObjectPtr<ExampleType> ExampleTypePtr;
  
  EmpiricalRisk(const PenalizationType& penalization, ObjectContainerPtr examples)
    : penalization(penalization), examples(examples) {}

  PenalizationType penalization;
  ObjectContainerPtr examples;
  
  enum {isDerivable = PenalizationType::isDerivable};
  
  void compute(const FeatureGeneratorPtr parameters, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    DenseVectorPtr denseParameters = parameters.dynamicCast<DenseVector>();
    jassert(denseParameters);
    
    if (output)
      *output = 0;
    if (!examples->size())
    {
      if (gradient)
        *gradient = emptyFeatureGenerator();
      return;
    }
    double Z = 1.0 / examples->size();
    
    std::vector<std::pair<FeatureGeneratorPtr, double> >* gradientLinearCombination = NULL;
    if (gradient)
    {
      gradientLinearCombination = new std::vector<std::pair<FeatureGeneratorPtr, double> >();
      gradientLinearCombination->reserve(examples->size());
    }

    for (size_t i = 0; i < examples->size(); ++i)
    {
      ExampleTypePtr example = examples->getAndCast<ExampleType>(i);
      const_cast<PenalizationType& >(penalization).right.setLearningExample(*example);

      // FIXME : gradient direction
      double lossOutput;
      FeatureGeneratorPtr lossGradient;
      penalization.compute(denseParameters, example->getInput(), output ? &lossOutput : NULL, gradient ? &lossGradient : NULL, NULL);
      double kZ = example->getWeight() * Z;
      if (output)
        *output += lossOutput * kZ;
      if (gradient)
        gradientLinearCombination->push_back(std::make_pair(lossGradient, kZ));
    }
    
    if (gradient)
      *gradient = linearCombination(gradientLinearCombination);
  }
};

/*
** Empirical risk with scalar outputs
*/
template<class ArchitectureType, class LossType, class ExampleType>
struct ScalarEmpiricalRisk : public EmpiricalRisk< 
      ScalarEmpiricalRisk<ArchitectureType, LossType, ExampleType> , 
      typename ScalarArchitectureScalarFunctionPair<ArchitectureType, LossType, void>::Composition,
      ExampleType
    >
{
  typedef ScalarEmpiricalRisk<ArchitectureType, LossType, ExampleType> ExactType;
  typedef typename ScalarArchitectureScalarFunctionPair<ArchitectureType, LossType, void>::Composition PenalizationType;
  typedef EmpiricalRisk<ExactType, PenalizationType, ExampleType> BaseClassType;
  
  ScalarEmpiricalRisk(const ArchitectureType& architecture, const LossType& loss, ObjectContainerPtr examples) 
    : BaseClassType(PenalizationType(architecture, loss), examples) {}
};

template<class ArchitectureType, class LossType, class ExampleType>
inline ScalarEmpiricalRisk<ArchitectureType, LossType, ExampleType>
  empiricalRisk(const ScalarArchitecture<ArchitectureType>& architecture, const ScalarLossFunction<LossType>& loss, ObjectContainerPtr examples, ExampleType* dummy)
  {return ScalarEmpiricalRisk<ArchitectureType, LossType, ExampleType >(static_cast<const ArchitectureType& >(architecture), static_cast<const LossType& >(loss), examples);}

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
  
  VectorEmpiricalRisk(const ArchitectureType& architecture, const LossType& loss, ObjectContainerPtr examples) 
    : BaseClassType(PenalizationType(architecture, loss), examples) {}
};

template<class ArchitectureType, class LossType, class ExampleType>
inline VectorEmpiricalRisk<ArchitectureType, LossType, ExampleType>
  empiricalRisk(const VectorArchitecture<ArchitectureType>& architecture, const VectorLossFunction<LossType>& loss, ObjectContainerPtr examples, ExampleType* dummy)
  {return VectorEmpiricalRisk<ArchitectureType, LossType, ExampleType >(static_cast<const ArchitectureType& >(architecture), static_cast<const LossType& >(loss), examples);}


}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_EMPIRICAL_RISK_H_
