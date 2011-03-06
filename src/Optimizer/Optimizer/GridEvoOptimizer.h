/*-----------------------------------------.---------------------------------.
 | Filename: GridEvoOptimizer.h             | Optimizer using Evolutionary    |
 | Author  : Arnaud Schoofs                 | Algorithm on Grid               |
 | Started : 01/03/2010 23:45               |                                 |
 `------------------------------------------/                                 |
                                            |                                 |
                                            `--------------------------------*/

#ifndef GRID_EVO_OPTIMIZER_H_
#define GRID_EVO_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../State/GridEvoOptimizerState.h"

// TODO arnaud : will contain common code for GridEvoOptimizer
// TODO arnaud : move file to include directory

namespace lbcpp
{

class GridEvoOptimizer : public Optimizer
{
public:
  virtual Variable optimize(ExecutionContext& context, const FunctionPtr& function, const DistributionPtr& apriori, const Variable& guess) const
  {
    // TODO arnaud
    // load state
    // goto ...
    
    return Variable();
  }
    
protected:
  friend class GridEvoOptimizerClass;
    
};

}; /* namespace lbcpp */

#endif // !GRID_EVO_OPTIMIZER_H_