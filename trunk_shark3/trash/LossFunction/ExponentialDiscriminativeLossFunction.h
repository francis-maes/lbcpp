/*-----------------------------------------.---------------------------------.
| Filename: ExponentialLossFunction.h      | Exponential Loss Function       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2011 17:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_EXPONENTIAL_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_EXPONENTIAL_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

// f(x) = exp(-input)
class ExponentialDiscriminativeLossFunction : public DiscriminativeLossFunction
{
public:
  virtual bool isDerivable() const
    {return true;}

  virtual void computeDiscriminativeLoss(double score, double* output, double* derivative) const
  {
    double e = exp(-score);
    if (output)
      *output = e;
    if (derivative)
      *derivative = -e;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_EXPONENTIAL_H_
