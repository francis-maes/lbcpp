/*-----------------------------------------.---------------------------------.
| Filename: ScalarToVectorArchitecture.hpp | Transform a scalar Architecture |
| Author  : Francis Maes                   |  into a vector Architecture     |
| Started : 16/03/2009 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_
# define LBCPP_CORE_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"
# include "../FeatureVisitor/FeatureVisitorStatic.hpp"

namespace lbcpp {
namespace impl {

template<class ScalarArchitectureType>
struct ScalarToVectorArchitecture
  : public VectorArchitecture< ScalarToVectorArchitecture<ScalarArchitectureType> >
{
  ScalarToVectorArchitecture(const ScalarArchitectureType& scalarArchitecture)
    : scalarArchitecture(scalarArchitecture) {}
    
  ScalarArchitectureType scalarArchitecture;
  
  enum {isDerivable = ScalarArchitectureType::isDerivable};
  
  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
    {return scalarArchitecture.createInitialParameters(inputDictionary, initializeRandomly);}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(outputNumber < input->getNumSubGenerators());
    scalarArchitecture.compute(parameters, input->getSubGeneratorWithIndex(outputNumber),
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
      res = new DenseVector(input->getDictionary()->getDictionaryWithSubScopesAsFeatures(), n);
    
    CompositeFeatureGeneratorPtr gParam, gInput;
    if (gradientWrtParameters)
      gParam = new CompositeFeatureGenerator(new FeatureDictionary(input->getDictionary()->getName(),
        StringDictionaryPtr(), input->getDictionary()->getScopes()), n);
    if (gradientWrtInput)
      gInput = new CompositeFeatureGenerator(new FeatureDictionary(input->getDictionary()->getName(),
        StringDictionaryPtr(), input->getDictionary()->getScopes()), n);
    
    for (size_t i = 0; i < n; ++i)
    {
      size_t index = input->getSubGeneratorIndex(i);
      double scalarOutput;
      FeatureGeneratorPtr gParamI, gInputI;
      scalarArchitecture.compute(parameters, input->getSubGenerator(i),
          output ? &scalarOutput : NULL, gParam ? &gParamI : NULL, gInput ? &gInputI : NULL);
      if (output)
        res->set(index, scalarOutput);
      if (gParam)
        gParam->setSubGenerator(index, gParamI);
      if (gInput)
        gInput->setSubGenerator(index, gInputI);
    }
    if (output)
      *output = res;
    if (gradientWrtParameters)
      *gradientWrtParameters = gParam;
    if (gradientWrtInput)
      *gradientWrtInput = gInput;
  }  
};

template<class ScalarArchitectureType>
inline ScalarToVectorArchitecture<ScalarArchitectureType> 
parallelArchitecture(const ScalarArchitecture<ScalarArchitectureType>& scalarArchitecture)
  {return ScalarToVectorArchitecture<ScalarArchitectureType>(static_cast<const ScalarArchitectureType& >(scalarArchitecture));}

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_SCALAR_TO_VECTOR_ARCHITECTURE_H_
