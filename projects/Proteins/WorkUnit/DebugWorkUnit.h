/*-----------------------------------------.---------------------------------.
| Filename: DebugWorkUnit.h                | WorkUnit used for debug purpose |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 05/04/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// TODO arnaud : useless file ?

#ifndef LBCPP_DEBUG_WORK_UNIT_H_
# define LBCPP_DEBUG_WORK_UNIT_H_

# include <lbcpp/lbcpp.h>
# include "../Optimizer/ProteinGridEvoOptimizer.h"

namespace lbcpp
{
class DebugWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context) 
  {
  
    ProteinGridEvoOptimizerStatePtr state = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml"))).staticCast<ProteinGridEvoOptimizerState>();
    foo(context, state);
    
    std::cout << numericalProteinPredictorParameters()->toString() << std::endl;
    
    return Variable();
  }
  
private:
  void foo(ExecutionContext& context, const GridOptimizerStatePtr& state_)
  {
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));
  }
};

};


#endif //!LBCPP_DEBUG_WORK_UNIT_H_
