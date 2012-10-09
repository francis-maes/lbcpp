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
  RandomOptimizer(SamplerPtr sampler, size_t numIterations = 0)
    : IterativeOptimizer(numIterations), sampler(sampler) {}
  RandomOptimizer() {}

  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeOptimizer::configure(context, problem, front, initialSolution, verbosity);
    sampler->initialize(context, problem->getDomain());
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    ObjectPtr object = sampleSolution(context, sampler); 
    FitnessPtr fitness = evaluate(context, object);
    if (verbosity >= Optimizer::verbosityDetailed)
    {
      context.resultCallback("object", object);
      context.resultCallback("fitness", fitness);
    }
    return true;
  }

protected:
  friend class RandomOptimizerClass;

  SamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_RANDOM_H_
