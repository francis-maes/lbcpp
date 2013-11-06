/*-----------------------------------------.---------------------------------.
 | Filename: EvolutionaryComparison.h      | Comparison of Evolutionary      |
 | Author  : Denny Verbeeck                | methods                         |
 | Started : 06/11/2013 11:15              |                                 |
 `-----------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EVOLUTIONARY_COMPARISON_H_
# define EVOLUTIONARY_COMPARISON_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/RandomVariable.h>
# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>

# include <ml/SplittingCriterion.h>
# include <ml/SelectionCriterion.h>

# include "SharkProblems.h"
# include "SolverInfo.h"

namespace lbcpp
{
  
extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class EvolutionaryComparison : public WorkUnit
{
  public:

  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    return ObjectPtr();
  }
  
protected:
  friend class SBOExperimentsClass;

}

}

#endif // !EVOLUTIONARY_COMPARISON_H_
