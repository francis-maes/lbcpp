/*-----------------------------------------.---------------------------------.
| Filename: RandomOptimizer.h              | Random Optimizer                |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_RANDOM_H_
# define LBCPP_ML_OPTIMIZER_RANDOM_H_

# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class RandomOptimizer : public IterativeOptimizer
{
public:
  RandomOptimizer(MOOSamplerPtr sampler, size_t numIterations = 0)
    : IterativeOptimizer(numIterations), sampler(sampler) {}
  RandomOptimizer() {}

  virtual void configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity)
  {
    IterativeOptimizer::configure(context, problem, front, verbosity);
    sampler->initialize(context, problem->getObjectDomain());
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
    {sampleAndEvaluateSolution(context, sampler); return true;}

protected:
  friend class RandomOptimizerClass;

  MOOSamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_RANDOM_H_
