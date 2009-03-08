/*-----------------------------------------.---------------------------------.
| Filename: ScalarDerivableArchitecture.hpp| Parameterized functions         |
| Author  : Francis Maes                   |   f_theta : Phi -> R            |
| Started : 07/03/2009 15:04               |      theta = DenseVector        |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_SCALAR_DERIVABLE_ARCHITECTURE_H_
# define CRALGO_IMPL_SCALAR_DERIVABLE_ARCHITECTURE_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

// f_theta(x) = dotProduct(x, theta)
struct LinearArchitecture : public ScalarArchitecture<LinearArchitecture>
{
  enum {isDerivable = true};
  
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

// f_theta(x) = theta_1
struct BiasArchitecture : public ScalarArchitecture<BiasArchitecture>
{
  enum {isDerivable = true};

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const
  {
    if (output)
      *output = parameters->get(0);
    if (gradientWrtParameters)
      gradientWrtParameters->set(0, 1.0);
    // gradientWrtInput : empty
  }
};

inline BiasArchitecture biasArchitecture()
  {return BiasArchitecture();}


}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_SCALAR_DERIVABLE_ARCHITECTURE_H_
