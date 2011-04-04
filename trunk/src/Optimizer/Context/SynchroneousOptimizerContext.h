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
  
  virtual juce::int64 evaluate(const Variable& parameters) 
  {
    const juce::int64 identifier = generateIdentifier();
    const Variable var = getObjectiveFunction()->compute(defaultExecutionContext(), parameters);
    getOptimizerCallback()->evaluationFinished(identifier, getDoubleFromOutput(var));
    return identifier;
  }

protected:  
  friend class SynchroneousOptimizerContextClass;

private:
  juce::int64 lastIdentifier; // TODO arnaud : static ?
  
  juce::int64 generateIdentifier()
  {
    juce::int64 res = Time::currentTimeMillis();
    if (res != lastIdentifier)
      return res;
    juce::Thread::sleep(1);
    return generateIdentifier();
  }

};
  
};
#endif // !LBCPP_SYNCHRONEOUS_OPTIMIZER_CONTEXT_H_