/*
 *  DebugWorkUnit.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 5/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

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
    
    
    NumericalProteinFeaturesParametersPtr parameters;
    std::cout << parameters->toString() << std::endl;
    
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