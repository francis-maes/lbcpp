/*-----------------------------------------.---------------------------------.
| Filename: BinarySumScalarVectorFunction.h| Binary Sum Scalar Vector Func   |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 15:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_BINARY_SUM_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_BINARY_SUM_H_

# include <lbcpp/Function/ScalarVectorFunction.h>

namespace lbcpp
{

class BinarySumScalarVectorFunction : public ScalarVectorFunction
{
public:
  BinarySumScalarVectorFunction(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2)
    : f1(f1), f2(f2) {}
  BinarySumScalarVectorFunction() {}

  virtual bool isDerivable() const
    {return f1->isDerivable() && f2->isDerivable();}

  virtual void compute(ExecutionContext& context, ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
  {
    f1->compute(context, input, output, gradientTarget, gradientWeight);
    f2->compute(context, input, output, gradientTarget, gradientWeight);
  }

  virtual void computeScalarVectorFunction(const DoubleVectorPtr& input, double* output, DoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    f1->computeScalarVectorFunction(input, output, gradientTarget, gradientWeight);
    f2->computeScalarVectorFunction(input, output, gradientTarget, gradientWeight);
  }

protected:
  friend class BinarySumScalarVectorFunctionClass;

  ScalarVectorFunctionPtr f1;
  ScalarVectorFunctionPtr f2;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_BINARY_SUM_H_
