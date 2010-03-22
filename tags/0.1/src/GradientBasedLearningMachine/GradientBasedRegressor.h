/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedRegressor.h       | Gradient based regressor        |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 14:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNING_MACHINE_REGRESSOR_H_
# define LBCPP_GRADIENT_BASED_LEARNING_MACHINE_REGRESSOR_H_

# include "StaticToDynamicGradientBasedMachine.h"

namespace lbcpp
{

class LeastSquaresLinearRegressor
  : public StaticToDynamicGradientBasedLearningMachine<LeastSquaresLinearRegressor, GradientBasedRegressor>
{
public:
  virtual ScalarArchitecturePtr getPredictionArchitecture() const
    {return impl::staticToDynamic(architecture());}

  inline impl::LinearArchitecture architecture() const
    {return impl::linearArchitecture();}

  inline impl::SquareLoss<RegressionExample> loss() const
    {return impl::squareLoss<RegressionExample>();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNING_MACHINE_REGRESSOR_H_
