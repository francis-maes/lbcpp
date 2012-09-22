/*-----------------------------------------.---------------------------------.
| Filename: SquareRegressionLossFunction.h | Square Loss Function            |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_SQUARE_REGRESSION_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_SQUARE_REGRESSION_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class SquareRegressionLossFunction : public RegressionLossFunction
{
public:
  virtual bool isDerivable() const
    {return true;}

  virtual void computeRegressionLoss(double input, double target, double* output, double* derivative) const
  {
    if (output)
    {
      double delta = input - target;
      *output = delta * delta;
    }
    if (derivative)
      *derivative = 2 * (input - target);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_SQUARE_REGRESSION_H_
