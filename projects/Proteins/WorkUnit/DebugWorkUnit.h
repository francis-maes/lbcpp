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
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include "../../../src/Distribution/Builder/GaussianDistributionBuilder.h"

# include "../Optimizer/ProteinGridEvoOptimizer.h"

namespace lbcpp
{
class DebugWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context) 
  {
    // variables used by DistributedOptimizerContext
    String projectName(T("DebugNetwork"));
    String source(T("arnaud@monster24"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("localhost"));
    size_t managerPort = 1664;
    size_t requiredMemory = 1;
    size_t requiredCpus = 1;
    size_t requiredTime = 1;
    
    // Optimizer
    OptimizerPtr optimizer = asyncEDAOptimizer(10000, 1000, 3, 30, 100, 1500);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, squareFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime);
    OptimizerStatePtr optimizerState = new OptimizerState();
    optimizerState->setDistribution(new GaussianDistribution(10, 10000));  // TODO arnaud use constructor from library
    return optimizer->compute(context, optimizerContext, optimizerState);
    
    
    /*ProteinGridEvoOptimizerStatePtr state = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml"))).staticCast<ProteinGridEvoOptimizerState>();
    foo(context, state);
    
    std::cout << numericalProteinPredictorParameters()->toString() << std::endl;
    
    return Variable();
  }
  
private:
  void foo(ExecutionContext& context, const GridOptimizerStatePtr& state_)
  {
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml")));*/
  }
};

};


#endif //!LBCPP_DEBUG_WORK_UNIT_H_
