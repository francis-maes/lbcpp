/*---------------------------------------.-----------------------------------------.
 | Filename: Utilites.h                  | Utility functions for experimental      |
 | Author  : Denny Verbeeck              | evaluation                              |
 | Started : 18/04/2014 11:25            |                                         |
 `---------------------------------------/                                         |
                                  |                                                |
                                  `-----------------------------------------------*/

#ifndef MOO_UTILITIES_H_
# define MOO_UTILITIES_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>

namespace lbcpp
{

typedef std::pair<ProblemPtr, string> ProblemAndDescription;
typedef std::pair<SolverPtr, string> SolverAndDescription;

class Utilities
{
public:
  static void runSolvers(ExecutionContext& context, ProblemAndDescription problem, std::vector<SolverAndDescription> solvers)
  {
    context.enterScope(problem.second);
    for (size_t i = 0; i < solvers.size(); ++i)
    {
      context.enterScope(solvers[i].second);

      context.leaveScope();
    }
    context.leaveScope();
  }

};

}

#endif //!MOO_UTILITIES_H_
