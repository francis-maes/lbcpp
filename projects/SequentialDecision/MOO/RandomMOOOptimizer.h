/*-----------------------------------------.---------------------------------.
| Filename: RandomMOOOptimizer.h           | Random Optimizer                |
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

class RandomMOOOptimizer : public MOOOptimizer
{
public:
  RandomMOOOptimizer(MOOSamplerPtr sampler, size_t numIterations)
    : sampler(sampler), numIterations(numIterations) {}
  RandomMOOOptimizer() : numIterations(0) {}

  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoSet)
  {
    for (size_t iteration = 0; iteration < numIterations && !problem->shouldStop(); ++iteration)
    {
      ObjectPtr solution = sampler->sample(context, problem->getSolutionDomain());
      paretoSet->insert(solution, problem->evaluate(context, solution));
    }
  }

protected:
  friend class RandomMOOOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_RANDOM_H_
