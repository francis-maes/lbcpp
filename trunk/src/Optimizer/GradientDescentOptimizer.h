/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOptimizer.h     | Gradient Descent Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_
# define LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_

# include <lbcpp/FeatureGenerator/Optimizer.h>
# include <lbcpp/Utilities/IterationFunction.h>

namespace lbcpp
{

class GradientDescentOptimizer : public VectorOptimizer
{
public:
  GradientDescentOptimizer(IterationFunctionPtr stepSize)
    : stepSize(stepSize) {}
  GradientDescentOptimizer() {}

  virtual String toString() const
    {return "GradientDescentOptimizer(" + lbcpp::toString(stepSize) + ")";}
  
  virtual OptimizerState step()
  {
    jassert(stepSize);
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
