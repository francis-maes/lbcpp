/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorDerivableFunc...hpp| Derivable functions f: R^d -> R |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_DERIVABLE_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_DERIVABLE_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

// f(x) = sum_i x_i^2
struct SumOfSquaresScalarVectorFunction : public ScalarVectorFunction< SumOfSquaresScalarVectorFunction >
{
  enum {isDerivable = true};
  
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr, LazyVectorPtr gradient) const
  {
    if (output)
      *output = input->sumOfSquares();
    if (gradient)
      gradient->addWeighted(input, 2.0);
  }
};

inline SumOfSquaresScalarVectorFunction sumOfSquares()
  {return SumOfSquaresScalarVectorFunction();}

// todo: ranking losses

}; /* namespace impl */
}; /* namespace cralgo */


#endif // !CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_DERIVABLE_H_
