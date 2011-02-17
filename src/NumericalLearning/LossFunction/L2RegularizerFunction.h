/*-----------------------------------------.---------------------------------.
| Filename: L2RegularizerFunction.h        | f(x) = sum_i x_i^2              |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_L2_REGULARIZER_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_L2_REGULARIZER_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class L2RegularizerFunction : public ScalarVectorFunction
{
public:
  L2RegularizerFunction(double weight = 0.0)
    : weight(weight) {}

  virtual String toString() const
    {return "(sum_i x_i^2)";}

  virtual bool isDerivable() const
    {return true;}

  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    if (output)
      *output += input->l2norm() * weight;
    if (gradientTarget)
    {
      if (input == *gradientTarget)
        (*gradientTarget)->multiplyByScalar(1 - 2.0 * gradientWeight * weight);
      else
        input->addWeightedTo(*gradientTarget, 0, gradientWeight * 2.0 * weight);
    }
  }

private:
  friend class L2RegularizerFunctionClass;

  double weight;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_L2_REGULARIZER_H_
