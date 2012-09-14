/*-----------------------------------------.---------------------------------.
| Filename: RandomOptimizer.h              | Random Optimizer                |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_OPTIMIZER_RANDOM_H_
# define LBCPP_MOO_OPTIMIZER_RANDOM_H_

# include "MOOCore.h"

namespace lbcpp
{

class RandomOptimizer : public MOOOptimizer
{
public:
  RandomOptimizer(MOOSamplerPtr sampler, size_t numIterations = 0)
    : sampler(sampler), numIterations(numIterations) {}
  RandomOptimizer() : numIterations(0) {}

  virtual void optimize(ExecutionContext& context)
  {
    sampler->initialize(context, problem->getObjectDomain());
    for (size_t iteration = 0; (!numIterations || iteration < numIterations) && !problem->shouldStop(); ++iteration)
      sampleAndEvaluateSolution(context, sampler);
  }

protected:
  friend class RandomOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_RANDOM_H_
