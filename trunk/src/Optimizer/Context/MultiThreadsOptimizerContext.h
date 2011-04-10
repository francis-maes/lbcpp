/*
 *  MultiThreadsOptimizerContext.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 3/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_
# define LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>

namespace lbcpp
{  
  
class MultiThreadsOptimizerContext : public OptimizerContext
{
public:
  MultiThreadsOptimizerContext(const FunctionPtr& objectiveFunction, size_t nbThreads = (size_t)juce::SystemStats::getNumCpus()) : OptimizerContext(objectiveFunction), nbThreads(nbThreads) 
    {MTContext = multiThreadedExecutionContext(nbThreads);}
  MultiThreadsOptimizerContext() {}
  
  virtual void waitAllEvaluationsFinished() const 
  {
    MTContext->waitUntilAllWorkUnitsAreDone();
  }
  
  virtual bool evaluate(const Variable& parameters) 
  {  
    WorkUnitPtr wu = new FunctionWorkUnit(objectiveFunction, parameters);
    MTContext->pushWorkUnit(wu);
    // callback is done in function evaluation !
    return true;
  }
  
protected:  
  friend class MultiThreadsOptimizerContextClass;
  
private:
  size_t nbThreads;
  ExecutionContextPtr MTContext;
  
};
  
};
#endif // !LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_
