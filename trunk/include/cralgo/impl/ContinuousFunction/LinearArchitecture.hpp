/*-----------------------------------------.---------------------------------.
| Filename: LinearArchitecture.hpp         | Linear Architecture             |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_LINEAR_ARCHITECTURE_H_
# define CRALGO_IMPL_FUNCTION_LINEAR_ARCHITECTURE_H_

# include "FunctionStatic.hpp"

namespace cralgo {
namespace impl {

// f_theta(x) = dotProduct(x, theta)
struct LinearArchitecture : public ScalarArchitecture<LinearArchitecture>
{
  enum {isDerivable = true};
  
  DenseVectorPtr createInitialParameters() const
    {return new DenseVector();}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const
  {
    if (output)
      *output = parameters->dotProduct(input);
    if (gradientWrtParameters)
      gradientWrtParameters->add(input);
    if (gradientWrtInput)
      gradientWrtInput->add(parameters);
  }
};

inline LinearArchitecture linearArchitecture()
  {return LinearArchitecture();}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_LINEAR_ARCHITECTURE_H_
