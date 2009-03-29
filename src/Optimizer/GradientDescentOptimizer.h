/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOptimizer.h     | Gradient Descent Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_OPTIMIZER_GRADIENT_DESCENT_H_
# define CRALGO_OPTIMIZER_GRADIENT_DESCENT_H_

# include <cralgo/Optimizer.h>
# include <cralgo/IterationFunction.h>

namespace cralgo
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

}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_GRADIENT_DESCENT_H_
