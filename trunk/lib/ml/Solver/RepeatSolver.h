/*-----------------------------------------.---------------------------------.
| Filename: RepeatSolver.h                 | Repeat a given solver multiple  |
| Author  : Francis Maes                   | times                           |
| Started : 22/10/2012 14:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_REPEAT_H_
# define LBCPP_ML_SOLVER_REPEAT_H_

# include <ml/Solver.h>

namespace lbcpp
{

class RepeatSolver : public IterativeSolver
{
public:
  RepeatSolver(SolverPtr solver, size_t numIterations = 0)
    : IterativeSolver(numIterations), solver(solver) {}
  RepeatSolver() {}
  
  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    solver->startSolver(context, problem, callback);
    solver->runSolver(context);
    solver->stopSolver(context);
    return true;
  }

protected:
  friend class RepeatSolverClass;

  SolverPtr solver;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_REPEAT_H_
