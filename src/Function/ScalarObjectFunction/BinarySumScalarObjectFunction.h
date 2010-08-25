/*-----------------------------------------.---------------------------------.
| Filename: BinarySumScalarObjectFunction.h| Binary Sum Scalar Vector Func   |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 15:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_BINARY_SUM_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_BINARY_SUM_H_

# include <lbcpp/Function/ScalarObjectFunction.h>

namespace lbcpp
{

class BinarySumScalarObjectFunction : public ScalarObjectFunction
{
public:
  BinarySumScalarObjectFunction(ScalarObjectFunctionPtr f1, ScalarObjectFunctionPtr f2)
    : f1(f1), f2(f2) {}
  BinarySumScalarObjectFunction() {}

  virtual bool isDerivable() const
    {return f1->isDerivable() && f2->isDerivable();}

  virtual void compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
  {
    f1->compute(input, output, gradientTarget, gradientWeight);
    f2->compute(input, output, gradientTarget, gradientWeight);
  }

protected:
  friend class BinarySumScalarObjectFunctionClass;

  ScalarObjectFunctionPtr f1;
  ScalarObjectFunctionPtr f2;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_BINARY_SUM_H_
