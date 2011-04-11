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
  
  virtual void waitUntilAllRequestsAreProcessed() const {}  // because evaluate is a blocking method
  
  // blocking method
  virtual bool evaluate(const Variable& parameters) 
  {  
    inProgressEvaluation = parameters;
    objectiveFunction->compute(defaultExecutionContext(), parameters);
    inProgressEvaluation = Variable();
    // callback is done in function evaluation !
    return true;
  }
  
protected:  
  friend class SynchroneousOptimizerContextClass;
  
private:
  Variable inProgressEvaluation;

};

};
#endif // !LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_
