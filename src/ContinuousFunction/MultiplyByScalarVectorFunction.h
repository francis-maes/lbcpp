/*-----------------------------------------.---------------------------------.
| Filename: MultiplyByScalarVectorFunction.h| VectorFunction * scalar        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 14:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VECTOR_FUNCTION_MULTIPLY_BY_SCALAR_H_
# define LBCPP_VECTOR_FUNCTION_MULTIPLY_BY_SCALAR_H_

# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

class MultiplyByScalarVectorFunction : public ScalarVectorFunction
{
public:
  MultiplyByScalarVectorFunction(ScalarVectorFunctionPtr function, double scalar)
    : function(function), scalar(scalar) {}

  virtual bool isDerivable() const
    {return function->isDerivable();}
  
  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    if (scalar == 0)
    {
      if (output)
        *output = 0;
      if (gradient)
        *gradient = emptyFeatureGenerator();
    }
    else
    {
      function->compute(input, output, gradientDirection && scalar < 0 ? lbcpp::multiplyByScalar(gradientDirection, -1.0) : gradientDirection, gradient);
      if (output) 
        *output *= scalar;
      if (gradient)
        *gradient = lbcpp::multiplyByScalar(*gradient, scalar);
    }
  }

protected:
  ScalarVectorFunctionPtr function;
  double scalar;
};

}; /* namespace lbcpp */

#endif // !LBCPP_VECTOR_FUNCTION_MULTIPLY_BY_SCALAR_H_
