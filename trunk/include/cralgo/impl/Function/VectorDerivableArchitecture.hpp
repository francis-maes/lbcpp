/*-----------------------------------------.---------------------------------.
| Filename: VectorDerivableArchitecture.hpp| Parameterized functions         |
| Author  : Francis Maes                   |   f_theta : Phi -> R^o          |
| Started : 07/03/2009 15:48               |      theta = DenseVector        |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_VECTOR_DERIVABLE_ARCHITECTURE_H_
# define CRALGO_IMPL_VECTOR_DERIVABLE_ARCHITECTURE_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

struct MultiLinearArchitecture : public VectorArchitecture< MultiLinearArchitecture >
{
  MultiLinearArchitecture(size_t numOutputs)
    : numOutputs(numOutputs) {}
    
  size_t numOutputs;
  
  DenseVectorPtr createInitialParameters() const
    {return new DenseVector(0, numOutputs);}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber, double* output, 
                LazyVectorPtr gradientWrtParameters,
                LazyVectorPtr gradientWrtInput) const
  {
    assert(outputNumber < numOutputs);
    DenseVectorPtr classParameters = parameters->getSubVector(outputNumber);
    if (output)
      *output = classParameters ? classParameters->dotProduct(input) : 0.0;
    if (gradientWrtParameters)
      gradientWrtParameters->set(input);
    if (gradientWrtInput)
      gradientWrtInput->set(classParameters);
  }
  
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                LazyVectorPtr output,
                LazyVectorPtr gradientWrtParameters,
                LazyVectorPtr gradientWrtInput) const
  {
    if (output)
    {
      DenseVectorPtr res = new DenseVector(numOutputs);
      for (size_t i = 0; i < numOutputs; ++i)
      {
        DenseVectorPtr classParameters = parameters->getSubVector(i);
        res->set(i, classParameters ? classParameters->dotProduct(input) : 0.0);
      }
      output->set(res);
    }
    if (gradientWrtParameters)
    {
      for (size_t i = 0; i < numOutputs; ++i)
      {
        LazyVectorPtr& subVector = gradientWrtParameters->getSubVector(i);
        if (!subVector)
          subVector = new LazyVector();
        subVector->add(input);
      }
    }
    if (gradientWrtInput)
      gradientWrtInput->set(parameters);
  }
};

inline MultiLinearArchitecture multiLinearArchitecture(size_t numOutputs)
  {return MultiLinearArchitecture(numOutputs);}

template<class DerivableFunction>
struct TransferArchitecture : public VectorArchitecture< TransferArchitecture<DerivableFunction> >
{
  TransferArchitecture(const DerivableFunction& function)
    : function(function) {}
  
  // todo: compute
  
  DerivableFunction function;
};

}; /* namespace impl */
}; /* namespace cralgo */


#endif // !CRALGO_IMPL_VECTOR_DERIVABLE_ARCHITECTURE_H_
