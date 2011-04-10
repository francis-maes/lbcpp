/*
 *  SynchroneousOptimizerContext.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 3/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_
# define LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Optimizer/OptimizerContext.h>

// TODO arnaud : replace defaultexecutioncontext

namespace lbcpp
{

class SynchroneousOptimizerContext : public OptimizerContext
{
public:
  SynchroneousOptimizerContext(const FunctionPtr& objectiveFunction) : OptimizerContext(objectiveFunction) {}
  SynchroneousOptimizerContext() {}
  
  virtual void waitAllEvaluationsFinished() const {}  // because evaluate is a blocking method
  
  // blocking method
  virtual bool evaluate(const Variable& parameters) 
  {    
    objectiveFunction->compute(defaultExecutionContext(), parameters);
    // callback is done in function !
    return true;
  }

protected:  
  friend class SynchroneousOptimizerContextClass;

};

};
#endif // !LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_
