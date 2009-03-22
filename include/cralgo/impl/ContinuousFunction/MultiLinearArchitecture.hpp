/*-----------------------------------------.---------------------------------.
| Filename: MultiLinearArchitecture.hpp    | Multiclass Linear Architecture  |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_
# define CRALGO_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace cralgo {
namespace impl {

struct MultiLinearArchitecture : public VectorArchitecture< MultiLinearArchitecture >
{
  MultiLinearArchitecture(FeatureDictionaryPtr outputs)
    : outputs(outputs) {}
    
  FeatureDictionaryPtr outputs;
  
  size_t getNumOutputs() const
    {return outputs->getFeatures().count();}
  
  DenseVectorPtr createInitialParameters() const
    {return new DenseVector(outputs, 0, getNumOutputs());}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(outputNumber < getNumOutputs());
    DenseVectorPtr classParameters = parameters->getSubVector(outputNumber);
    if (output)
      *output = classParameters ? classParameters->dotProduct(input) : 0.0;
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
      DenseVectorPtr res = new DenseVector(outputs, numOutputs);
      for (size_t i = 0; i < numOutputs; ++i)
      {
        DenseVectorPtr classParameters = parameters->getSubVector(i);
        res->set(i, classParameters ? classParameters->dotProduct(input) : 0.0);
      }
      *output = res;
    }
    if (gradientWrtParameters)
    {
      // parameters gradient of output i depends linearly on class parameters i
      // e.g.
      // 0.0.input
      // 1.1.input
      // 2.2.input (la sortie 2 ne dépend que du sous-vecteurs de paramètres 2 linéairement en fonction de l'entrée)
      CompositeFeatureGeneratorPtr g = new CompositeFeatureGenerator(numOutputs, new FeatureDictionary("pouet")); // FIXME: dictionary
      FeatureDictionaryPtr pouet = new FeatureDictionary("pouet2");
      for (size_t i = 0; i < numOutputs; ++i)
        g->setSubGenerator(i, FeatureGenerator::subFeatureGenerator(pouet, i, input)); // FIXME: dictionary
      *gradientWrtParameters = g;
    }
    if (gradientWrtInput)
      *gradientWrtInput = parameters;
  }
};

inline MultiLinearArchitecture multiLinearArchitecture(FeatureDictionaryPtr outputs)
  {return MultiLinearArchitecture(outputs);}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_MULTI_LINEAR_ARCHITECTURE_H_
