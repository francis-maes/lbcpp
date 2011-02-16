/*-----------------------------------------.---------------------------------.
| Filename: SquareLossFunction.h           | Square Loss Function            |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_SQUARE_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_SQUARE_H_

# include <lbcpp/NumericalLearning/LossFunctions.h>

namespace lbcpp
{

class SquareLossFunction : public RegressionLossFunction
{
public:
  SquareLossFunction(double target = 0.0)
    : RegressionLossFunction(target) {}

  virtual bool isDerivable() const
    {return true;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
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

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_SQUARE_H_
