/*-----------------------------------------.---------------------------------.
| Filename: BiasArchitecture.hpp           | One parameter: the bias         |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_BIAS_ARCHITECTURE_H_
# define CRALGO_IMPL_BIAS_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace cralgo {
namespace impl {

// f_theta(x) = theta_1
struct BiasArchitecture : public ScalarArchitecture<BiasArchitecture>
{
  enum {isDerivable = true};

  DenseVectorPtr createInitialParameters() const
    {return new DenseVector(getDictionary(), 1);}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const
  {
    if (output)
      *output = parameters->get(0);
    if (gradientWrtParameters)
    {
      DenseVectorPtr g = new DenseVector(getDictionary());
      g->set(0, 1.0);
      *gradientWrtParameters = g;
    }
    // gradientWrtInput : empty
  }
  
  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr biasDictionary = new FeatureDictionary("bias");
    if (biasDictionary->empty())
      biasDictionary->getFeatures().add("bias");
    return biasDictionary;
  }
};

inline BiasArchitecture biasArchitecture()
  {return BiasArchitecture();}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_BIAS_ARCHITECTURE_H_
