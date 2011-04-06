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
  
// TODO arnaud : move elsewhere  
class FunctionWithOptimizerCallbackAndStateWorkUnit : public WorkUnit
{
public:
  FunctionWithOptimizerCallbackAndStateWorkUnit(const FunctionPtr& function, const Variable input, const OptimizerCallbackPtr& optimizerCallback, const OptimizerStatePtr& optimizerState)
  : function(function), input(input), optimizerCallback(optimizerCallback), optimizerState(optimizerState) {}
  
  FunctionWithOptimizerCallbackAndStateWorkUnit() {}
  
  virtual Variable run(ExecutionContext& context) 
  {
    Variable output = function->compute(context, input);
    optimizerCallback->evaluationFinished(input, getDoubleFromOutput(output), optimizerState);
    return output;
  }
  
protected:
  friend class FunctionWithOptimizerCallbackAndStateWorkUnitClass;
  
private:
  FunctionPtr function;
  Variable input;
  OptimizerCallbackPtr optimizerCallback;
  OptimizerStatePtr optimizerState;
  
  
  // TODO arnaud : method is already in OptimizerContext ...
  double getDoubleFromOutput(const Variable& variable)
  {
    if (variable.isDouble())
      return variable.getDouble();
    
    if (!variable.isObject()) {
      jassertfalse;
      return 1;
    }
    
    ObjectPtr object = variable.getObject();
    
    if (!object->getClass()->inheritsFrom(scoreObjectClass)) {
      jassertfalse;
      return 1;
    }
    
    return object.staticCast<ScoreObject>()->getScoreToMinimize();
    
  }
};
  
  
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
  
  // blocking method
  virtual bool evaluate(const Variable& parameters, const OptimizerStatePtr& optimizerState) 
  {  
    WorkUnitPtr wu = new FunctionWithOptimizerCallbackAndStateWorkUnit(getObjectiveFunction(), parameters, getOptimizerCallback(), optimizerState);
    MTContext->pushWorkUnit(wu);
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