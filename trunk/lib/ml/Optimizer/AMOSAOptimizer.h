/*-----------------------------------------.---------------------------------.
| Filename: AMOSAOptimizer.h               | AMOSA Optimizer                 |
| Author  : Denny Verbeeck                 |                                 |
| Started : 20/02/2014 16:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_OPTIMIZER_AMOSA_H_
# define ML_OPTIMIZER_AMOSA_H_

# include <ml/Problem.h>
# include <ml/Sampler.h>
# include <ml/Solver.h>
# include <ml/SolutionContainer.h>
# include <ml/SolutionComparator.h>

namespace lbcpp
{

class AMOSAOptimizer : public PopulationBasedSolver
{
public:
  virtual void startSolver(ExecutionContext& context) {}

protected:
  size_t hardLimit;   /**< hard limit for the archive */
  size_t softLimit;   /**< soft limit for the archive */
  size_t numIter;     /**< number of iterations per temperature setting */
  size_t hillClimb;   /**< number of hillclimbing iterations */
  double tMin;        /**< minimum temperature */
  double coolingRate; /**< cooling rate */
 
};

}; /* namespace lbcpp */

#endif //!ML_OPTIMIZER_AMOSA_H_
