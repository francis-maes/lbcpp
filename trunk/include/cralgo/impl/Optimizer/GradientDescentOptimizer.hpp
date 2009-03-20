/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOptimizer.hpp   | Gradient Descent Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_OPTIMIZER_GRADIENT_DESCENT_H_
# define CRALGO_IMPL_OPTIMIZER_GRADIENT_DESCENT_H_

# include "OptimizerStatic.hpp"
# include "../../IterationFunction.h"

namespace cralgo {
namespace impl {

struct GradientDescentOptimizer : public VectorOptimizer<GradientDescentOptimizer>
{
  GradientDescentOptimizer(IterationFunctionPtr stepSize)
    : stepSize(stepSize) {}
  
  DenseVectorPtr optimizerStart(ScalarVectorFunctionPtr function, DenseVectorPtr parameters)
  {
    iteration = 0;
    return parameters;
  }

  DenseVectorPtr optimizerStep(ScalarVectorFunctionPtr function, DenseVectorPtr parameters, const DenseVectorPtr gradient, double value)
  {
    parameters->addWeighted(gradient, -(stepSize->compute(iteration++)));
    return parameters;
  }
  
private:
  size_t iteration;
  IterationFunctionPtr stepSize;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_OPTIMIZER_GRADIENT_DESCENT_H_
