/*-----------------------------------------.---------------------------------.
| Filename: SumOfSquaresScalarVectorFunction.h| f(x) = sum_i x_i^2           |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VECTOR_FUNCTION_SUM_OF_SQUARES_H_
# define LBCPP_VECTOR_FUNCTION_SUM_OF_SQUARES_H_

# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

class SumOfSquaresScalarVectorFunction : public ScalarVectorFunction
{
public:
  virtual String toString() const
    {return "(sum_i x_i^2)";}

  virtual bool isDerivable() const
    {return true;}
  
  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
     if (output)
      *output = input->sumOfSquares();
    if (gradient)
      *gradient = lbcpp::multiplyByScalar(input, 2.0);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_VECTOR_FUNCTION_SUM_OF_SQUARES_H_
