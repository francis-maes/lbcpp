/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOptimizer.h     | Gradient Descent Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_
# define LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_

# include <lbcpp/Optimizer.h>
# include <lbcpp/IterationFunction.h>

namespace lbcpp
{

class GradientDescentOptimizer : public VectorOptimizer
{
public:
  GradientDescentOptimizer(IterationFunctionPtr stepSize)
    : stepSize(stepSize) {}
  
  virtual OptimizerState step()
  {
    DenseVectorPtr denseParameters = parameters->toDenseVector();
    denseParameters->addWeighted(gradient, -(stepSize->compute(iteration)));
    setParameters(denseParameters);
    return optimizerContinue;
  }
  
private:
  IterationFunctionPtr stepSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_
