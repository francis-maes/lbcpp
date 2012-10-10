/*-----------------------------------------.---------------------------------.
| Filename: RandomOptimizer.h              | Random Optimizer                |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_RANDOM_H_
# define LBCPP_ML_OPTIMIZER_RANDOM_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class RandomOptimizer : public IterativeSolver
{
public:
  RandomOptimizer(SamplerPtr sampler, size_t numIterations = 0)
    : IterativeSolver(numIterations), sampler(sampler) {}
  RandomOptimizer() {}

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);
    sampler->initialize(context, problem->getDomain());
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    ObjectPtr object = sampler->sample(context);
    FitnessPtr fitness = evaluate(context, object);
    if (verbosity >= verbosityDetailed)
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
