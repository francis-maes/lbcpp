/*-----------------------------------------.---------------------------------.
| Filename: MultiLinearArchitecture.hpp    | Multiclass Linear Architecture  |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_
# define LBCPP_CORE_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace lbcpp {
namespace impl {

struct MultiLinearArchitecture : public VectorArchitecture< MultiLinearArchitecture >
{
  MultiLinearArchitecture(FeatureDictionaryPtr outputs = FeatureDictionaryPtr())
  {
    if (outputs)
      setOutputs(outputs);
  }
  
  void setOutputs(FeatureDictionaryPtr outputs)
    {outputsDictionary = outputs;}
    
  size_t getNumOutputs() const
    {jassert(outputsDictionary); return outputsDictionary->getNumFeatures();}
  
  FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {return FeatureDictionaryManager::getInstance().getCollectionDictionary(outputsDictionary->getFeatures(), inputDictionary);}
  
  DenseVectorPtr getClassParameters(DenseVectorPtr parameters, size_t outputNumber, FeatureDictionaryPtr inputDictionary) const
  {
    DenseVectorPtr& res = parameters->getSubVector(outputNumber);
    if (!res)
    {
      res = new DenseVector(inputDictionary);
      parameters->getDictionary()->setSubDictionary(outputNumber, inputDictionary);
    }
    return res;
  }

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    jassert(outputNumber < getNumOutputs());
    DenseVectorPtr classParameters = getClassParameters(parameters, outputNumber, input->getDictionary());
    if (output)
      *output = classParameters->dotProduct(input);
    if (gradientWrtParameters)
      *gradientWrtParameters = subFeatureGenerator(parameters->getDictionary(), outputNumber, input);
    if (gradientWrtInput)
      *gradientWrtInput = subFeatureGenerator(input->getDictionary(), outputNumber, classParameters);
  }
  
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                FeatureGeneratorPtr* output,
                FeatureGeneratorPtr* gradientsWrtParameters,
                FeatureGeneratorPtr* gradientsWrtInput) const
  {
    size_t numOutputs = getNumOutputs();
    if (output)
    {
      DenseVectorPtr res = new DenseVector(outputsDictionary, numOutputs);
      for (size_t i = 0; i < numOutputs; ++i)
      {
        DenseVectorPtr classParameters = getClassParameters(parameters, i, input->getDictionary());
        res->set(i, classParameters->dotProduct(input));
      }
      jassert(res->getNumValues() == numOutputs);
      *output = res;
    }
    if (gradientsWrtParameters)
    {
      // parameters gradient of output i depends linearly on class parameters i
      // e.g.
      // 0.0.input
      // 1.1.input
      // 2.2.input (la sortie 2 ne dépend que du sous-vecteurs de paramètres 2 linéairement en fonction de l'entrée)
      FeatureDictionaryPtr collectionOfGradientsDictionary = FeatureDictionaryManager::getInstance().getCollectionDictionary(outputsDictionary->getFeatures(), parameters->getDictionary());
      CompositeFeatureGeneratorPtr g = new CompositeFeatureGenerator(collectionOfGradientsDictionary, numOutputs);
      for (size_t i = 0; i < numOutputs; ++i)
        g->setSubGenerator(i, subFeatureGenerator(parameters->getDictionary(), i, input));
      *gradientsWrtParameters = g;
    }
    if (gradientsWrtInput)
      *gradientsWrtInput = parameters;
  }
  
private:
  FeatureDictionaryPtr outputsDictionary;
};

inline MultiLinearArchitecture multiLinearArchitecture(FeatureDictionaryPtr outputs)
  {return MultiLinearArchitecture(outputs);}

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LBCPP_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_
