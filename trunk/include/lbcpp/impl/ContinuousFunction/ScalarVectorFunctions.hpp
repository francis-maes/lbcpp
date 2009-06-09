/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorFunctions.hpp      | Functions from vectors to       |
| Author  : Francis Maes                   |     scalars f: R^d -> R         |
| Started : 11/03/2009 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_SCALAR_VECTOR_FUNCTIONS_HPP_
# define LBCPP_CORE_IMPL_FUNCTION_SCALAR_VECTOR_FUNCTIONS_HPP_

# include "FunctionStatic.hpp"

namespace lbcpp {
namespace impl {


// f(x) = sum_i x_i^2
struct SumOfSquaresScalarVectorFunction : public ScalarVectorFunction< SumOfSquaresScalarVectorFunction >
{
  enum {isDerivable = true};
  
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr, FeatureGeneratorPtr* gradient) const
  {
    if (output)
      *output = input->sumOfSquares();
    if (gradient)
      *gradient = multiplyByScalar(input, 2.0);
  }
  
  std::string toString() const
    {return "(sum_i x_i^2)";}
};

inline SumOfSquaresScalarVectorFunction sumOfSquares()
  {return SumOfSquaresScalarVectorFunction();}


}; /* namespace impl */
}; /* namespace lbcpp */


#endif // !LBCPP_CORE_IMPL_FUNCTION_SCALAR_VECTOR_FUNCTIONS_HPP_
