/*-----------------------------------------.---------------------------------.
| Filename: ScalarToVectorArchitecture.hpp | Transform a scalar Architecture |
| Author  : Francis Maes                   |  into a vector Architecture     |
| Started : 16/03/2009 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"
# include "../FeatureVisitor/FeatureVisitorStatic.hpp"

namespace cralgo {
namespace impl {

template<class ScalarArchitectureType>
struct ScalarToVectorArchitecture
  : public VectorArchitecture< ScalarToVectorArchitecture<ScalarArchitectureType> >
{
  ScalarToVectorArchitecture(const ScalarArchitectureType& scalarArchitecture)
    : scalarArchitecture(scalarArchitecture) {}
    
  ScalarArchitectureType scalarArchitecture;
  
  enum {isDerivable = ScalarArchitectureType::isDerivable};
  
  DenseVectorPtr createInitialParameters() const
    {return scalarArchitecture.createInitialParameters();}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber, double* output, 
                LazyVectorPtr gradientWrtParameters,
                LazyVectorPtr gradientWrtInput) const
  {
    CompositeFeatureGeneratorPtr inputs = input.dynamicCast<CompositeFeatureGenerator>();
    assert(inputs); // for the moment, ScalarToVectorArchitecture only works with composite feature generators
    assert(outputNumber < inputs->getNumFeatureGenerators());
    scalarArchitecture.compute(parameters, inputs->getFeatureGenerator(outputNumber),
      output, gradientWrtParameters, gradientWrtInput);
  }
    
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                LazyVectorPtr output,
                LazyVectorPtr gradientWrtParameters,
                LazyVectorPtr gradientWrtInput) const
  {
    CompositeFeatureGeneratorPtr inputs = input.dynamicCast<CompositeFeatureGenerator>();
    assert(inputs); // for the moment, ScalarToVectorArchitecture only works with composite feature generators
    
    DenseVectorPtr res;
    if (output)
      res = new DenseVector(inputs->getNumFeatureGenerators());
    
    for (size_t i = 0; i < inputs->getNumFeatureGenerators(); ++i)
    {
      double scalarOutput;
      LazyVectorPtr gParam, gInput;
      if (gradientWrtParameters)
        gParam = new LazyVector();
      if (gradientWrtInput)
        gInput = new LazyVector();
      scalarArchitecture.compute(parameters, inputs->getFeatureGenerator(i),
          output ? &scalarOutput : NULL, gParam, gInput);
      if (output)
        res->set(i, scalarOutput);
      if (gParam)
        gradientWrtParameters->setSubVector(i, gParam);
      if (gInput)
        gradientWrtInput->setSubVector(i, gInput);
    }
    if (output)
      output->set(res);
  }  
};

template<class ScalarArchitectureType>
inline ScalarToVectorArchitecture<ScalarArchitectureType> 
parallelArchitecture(const ScalarArchitecture<ScalarArchitectureType>& scalarArchitecture)
  {return ScalarToVectorArchitecture<ScalarArchitectureType>(static_cast<const ScalarArchitectureType& >(scalarArchitecture));}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_
