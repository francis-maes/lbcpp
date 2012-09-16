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

class RandomOptimizer : public IterativeOptimizer
{
public:
  RandomOptimizer(MOOSamplerPtr sampler, size_t numIterations = 0)
    : IterativeOptimizer(numIterations), sampler(sampler) {}
  RandomOptimizer() {}

  virtual bool iteration(ExecutionContext& context, size_t iter)
    {sampleAndEvaluateSolution(context, sampler); return true;}

  virtual void optimize(ExecutionContext& context)
  {
    sampler->initialize(context, problem->getObjectDomain());
    IterativeOptimizer::optimize(context);
  }

protected:
  friend class RandomOptimizerClass;

  MOOSamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_RANDOM_H_
