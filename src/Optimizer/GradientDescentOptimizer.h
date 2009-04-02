/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOptimizer.h     | Gradient Descent Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_OPTIMIZER_GRADIENT_DESCENT_H_
# define LCPP_OPTIMIZER_GRADIENT_DESCENT_H_

# include <lcpp/Optimizer.h>
# include <lcpp/IterationFunction.h>

namespace lcpp
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

}; /* namespace lcpp */

#endif // !LCPP_OPTIMIZER_GRADIENT_DESCENT_H_
