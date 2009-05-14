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
  
  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
  {
    DenseVectorPtr res = new DenseVector(inputDictionary);
    if (initializeRandomly)
      res->initializeRandomly();
    return res;
  }

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const
  {
    if (output)
      *output = parameters->dotProduct(input);
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
