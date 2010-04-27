/*-----------------------------------------.---------------------------------.
| Filename: BiasArchitecture.hpp           | One parameter: the bias         |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_BIAS_ARCHITECTURE_H_
# define LBCPP_CORE_IMPL_BIAS_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace lbcpp {
namespace impl {

// f_theta(x) = theta_1
struct BiasArchitecture : public ScalarArchitecture<BiasArchitecture>
{
  enum {isDerivable = true};

  FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {return getDictionary();}

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
    if (gradientWrtInput)
      *gradientWrtInput = emptyFeatureGenerator();
  }
  
  static FeatureDictionaryPtr getDictionary()
  {
    static FeatureDictionaryPtr dictionary;
    if (!dictionary)
      dictionary = FeatureDictionaryManager::getInstance().getOrCreateRootDictionary(T("bias"), true, false);
    return dictionary;
  }
};

inline BiasArchitecture biasArchitecture()
  {return BiasArchitecture();}

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_BIAS_ARCHITECTURE_H_
