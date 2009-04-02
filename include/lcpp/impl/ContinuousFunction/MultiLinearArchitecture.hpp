/*-----------------------------------------.---------------------------------.
| Filename: MultiLinearArchitecture.hpp    | Multiclass Linear Architecture  |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_
# define LCPP_CORE_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace lcpp {
namespace impl {

struct MultiLinearArchitecture : public VectorArchitecture< MultiLinearArchitecture >
{
  MultiLinearArchitecture(StringDictionaryPtr outputs)
    : outputs(outputs),
      paramsDictionary(new FeatureDictionary("MultiLinearArchitecture parameters", StringDictionaryPtr(), outputs)),
      outputsDictionary(new FeatureDictionary("MultiLinearArchitecture outputs", outputs, StringDictionaryPtr())),
      paramsGradientDictionary(new FeatureDictionary("MultiLinearArchitecture gradient wrt parameters", StringDictionaryPtr(), outputs))
  {
    classParamsGradientDictionary = new FeatureDictionary("MultiLinearArchitecture gradient wrt class-parameters", StringDictionaryPtr(), outputs);
    for (size_t i = 0; i < outputs->getNumElements(); ++i)
      paramsGradientDictionary->setSubDictionary(i, classParamsGradientDictionary);
  }
    
  size_t getNumOutputs() const
    {assert(outputs->getNumElements()); return outputs->getNumElements();}
  
  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
  {
    DenseVectorPtr res = new DenseVector(paramsDictionary, 0, getNumOutputs());
    for (size_t i = 0; i < outputs->getNumElements(); ++i)
    {
      DenseVectorPtr classParams = new DenseVector(inputDictionary);
      if (initializeRandomly)
        classParams->initializeRandomly();
      res->setSubVector(i, classParams);
    }
    return res;
  }

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(outputNumber < getNumOutputs());
    DenseVectorPtr& classParameters = parameters->getSubVector(outputNumber);
    if (!classParameters)
      classParameters = new DenseVector(input->getDictionary());
    if (output)
      *output = classParameters->dotProduct(input);
    if (gradientWrtParameters)
      *gradientWrtParameters = input;
    if (gradientWrtInput)
      *gradientWrtInput = classParameters;
  }
  
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                FeatureGeneratorPtr* output,
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    size_t numOutputs = getNumOutputs();
    if (output)
    {
      DenseVectorPtr res = new DenseVector(outputsDictionary, numOutputs);
      for (size_t i = 0; i < numOutputs; ++i)
      {
        DenseVectorPtr& classParameters = parameters->getSubVector(i);
        if (!classParameters)
          classParameters = new DenseVector(input->getDictionary());
        res->set(i, classParameters->dotProduct(input));
      }
      assert(res->getNumValues() == numOutputs);
      *output = res;
    }
    if (gradientWrtParameters)
    {
      // parameters gradient of output i depends linearly on class parameters i
      // e.g.
      // 0.0.input
      // 1.1.input
      // 2.2.input (la sortie 2 ne dépend que du sous-vecteurs de paramètres 2 linéairement en fonction de l'entrée)
      CompositeFeatureGeneratorPtr g = new CompositeFeatureGenerator(paramsGradientDictionary, numOutputs);
      for (size_t i = 0; i < numOutputs; ++i)
        g->setSubGenerator(i, FeatureGenerator::subFeatureGenerator(classParamsGradientDictionary, i, input));
      *gradientWrtParameters = g;
    }
    if (gradientWrtInput)
      *gradientWrtInput = parameters;
  }
  
private:
  StringDictionaryPtr outputs;
  FeatureDictionaryPtr paramsDictionary, outputsDictionary;
  FeatureDictionaryPtr paramsGradientDictionary, classParamsGradientDictionary;
};

inline MultiLinearArchitecture multiLinearArchitecture(StringDictionaryPtr outputs)
  {return MultiLinearArchitecture(outputs);}

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_
