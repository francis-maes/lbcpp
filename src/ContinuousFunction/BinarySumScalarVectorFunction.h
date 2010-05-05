/*-----------------------------------------.---------------------------------.
| Filename: BinarySumScalarVectorFunction.h| Binary Sum Scalar Vector Func   |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 15:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_BINARY_SUM_SCALAR_VECTOR_FUNCTION_H_
# define LBCPP_CONTINUOUS_FUNCTION_BINARY_SUM_SCALAR_VECTOR_FUNCTION_H_

# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

class BinarySumScalarVectorFunction : public ScalarVectorFunction
{
public:
  BinarySumScalarVectorFunction(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2)
    : f1(f1), f2(f2) {}

  virtual bool isDerivable() const
    {return f1->isDerivable() && f2->isDerivable();}

  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    double output1, output2;
    FeatureGeneratorPtr gradient1, gradient2;
    f1->compute(input, output ? &output1 : NULL, gradientDirection, gradient ? &gradient1 : NULL);
    f2->compute(input, output ? &output2 : NULL, gradientDirection, gradient ? &gradient2 : NULL);
    if (output)
      *output = output1 + output2;
    if (gradient)
      *gradient = weightedSum(gradient1, 1.0, gradient2, 1.0);
  }

protected:
  ScalarVectorFunctionPtr f1;
  ScalarVectorFunctionPtr f2;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_BINARY_SUM_SCALAR_VECTOR_FUNCTION_H_
