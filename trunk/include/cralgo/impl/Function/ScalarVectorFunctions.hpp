/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorFunctions.hpp      | Functions from vectors to       |
| Author  : Francis Maes                   |     scalars f: R^d -> R         |
| Started : 11/03/2009 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_VECTOR_FUNCTIONS_HPP_
# define CRALGO_IMPL_FUNCTION_SCALAR_VECTOR_FUNCTIONS_HPP_

# include "FunctionStatic.hpp"
# include "LossFunctions.hpp"

namespace cralgo
{
namespace impl
{

// f(x) = abs(x)
struct AbsoluteScalarFunction : public ScalarFunction<AbsoluteScalarFunction>
{
  enum {isDerivable = false};

  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = fabs(input);
    if (derivative)
    {
      if (input > 0)
        *derivative = 1;
      else if (input < 0)
        *derivative = -1;
      else
      {
        assert(derivativeDirection);
        if (*derivativeDirection > 0)
          *derivative = 1;
        else if (*derivativeDirection < 0)
          *derivative = -1;
        else
          *derivative = 0;
      }
    }
  }
};
inline AbsoluteScalarFunction absFunction()
  {return AbsoluteScalarFunction();}

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

STATIC_REGRESSION_LOSS_FUNCTION(squareLoss, SquareLoss, SquareScalarFunction);
STATIC_REGRESSION_LOSS_FUNCTION(absoluteLoss, AbsoluteLoss, AbsoluteScalarFunction);

}; /* namespace impl */
}; /* namespace cralgo */


#endif // !CRALGO_IMPL_FUNCTION_SCALAR_VECTOR_FUNCTIONS_HPP_
