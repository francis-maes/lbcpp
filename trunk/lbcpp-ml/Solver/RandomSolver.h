/*-----------------------------------------.---------------------------------.
| Filename: RandomSolver.h                 | Random Solver                   |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_RANDOM_H_
# define LBCPP_ML_SOLVER_RANDOM_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class RandomSolver : public IterativeSolver
{
public:
  RandomSolver(SamplerPtr sampler, size_t numIterations = 0)
    : IterativeSolver(numIterations), sampler(sampler) {}
  RandomSolver() {}

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
  friend class RandomSolverClass;

  SamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_RANDOM_H_
