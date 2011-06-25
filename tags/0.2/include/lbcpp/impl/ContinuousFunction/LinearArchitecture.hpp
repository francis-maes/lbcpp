/*-----------------------------------------.---------------------------------.
| Filename: LinearArchitecture.hpp         | Linear Architecture             |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_LINEAR_ARCHITECTURE_H_
# define LBCPP_CORE_IMPL_FUNCTION_LINEAR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace lbcpp {
namespace impl {

// f_theta(x) = dotProduct(x, theta)
struct LinearArchitecture : public ScalarArchitecture<LinearArchitecture>
{
  enum {isDerivable = true};
  
  FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {return inputDictionary;}
    
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const
  {
    if (output)
      *output = parameters->dotProduct(input, dotProductCache);
    if (gradientWrtParameters)
      *gradientWrtParameters = input;
    if (gradientWrtInput)
      *gradientWrtInput = parameters;
  }
};

inline LinearArchitecture linearArchitecture()
  {return LinearArchitecture();}

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_LINEAR_ARCHITECTURE_H_