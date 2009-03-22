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
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(outputNumber < input->getNumSubGenerators());
    scalarArchitecture.compute(parameters, input->getSubGenerator(outputNumber),
      output, gradientWrtParameters, gradientWrtInput);
  }
    
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                FeatureGeneratorPtr* output,
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    size_t n = input->getNumSubGenerators();
    
    DenseVectorPtr res;
    if (output)
      res = new DenseVector(n);
    
    CompositeFeatureGeneratorPtr gParam, gInput;
    if (gradientWrtParameters)
      gParam = new CompositeFeatureGenerator(n), *gradientWrtParameters = gParam;
    if (gradientWrtInput)
      gInput = new CompositeFeatureGenerator(n), *gradientWrtInput = gInput;
    
    for (size_t i = 0; i < n; ++i)
    {
      double scalarOutput;
      FeatureGeneratorPtr gParamI, gInputI;
      scalarArchitecture.compute(parameters, input->getSubGenerator(i),
          output ? &scalarOutput : NULL, gParam ? &gParamI : NULL, gInput ? &gInputI : NULL);
      if (output)
        res->set(i, scalarOutput);
      if (gParam)
        gParam->setSubGenerator(i, gParamI);
      if (gInput)
        gInput->setSubGenerator(i, gInputI);
    }
    if (output)
      *output = res;
  }  
};

template<class ScalarArchitectureType>
inline ScalarToVectorArchitecture<ScalarArchitectureType> 
parallelArchitecture(const ScalarArchitecture<ScalarArchitectureType>& scalarArchitecture)
  {return ScalarToVectorArchitecture<ScalarArchitectureType>(static_cast<const ScalarArchitectureType& >(scalarArchitecture));}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_
