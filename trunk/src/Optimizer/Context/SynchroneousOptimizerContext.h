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
  
  // blocking method
  virtual bool evaluate(const Variable& parameters, const OptimizerStatePtr& optimizerState) 
  {    
    const Variable var = getObjectiveFunction()->compute(defaultExecutionContext(), parameters);
    getOptimizerCallback()->evaluationFinished(parameters, getDoubleFromOutput(var), optimizerState);
    return true;
  }

protected:  
  friend class SynchroneousOptimizerContextClass;

private:
  /*juce::int64 lastIdentifier; // TODO arnaud : static ?
  
  juce::int64 generateIdentifier()
  {
    juce::int64 res = Time::currentTimeMillis();
    if (res != lastIdentifier)
      return res;
    juce::Thread::sleep(1);
    return generateIdentifier();
  }*/

};
  
};
#endif // !LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_