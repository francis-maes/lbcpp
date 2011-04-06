/*
 *  OptimizerContext.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 3/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_OPTIMIZER_CONTEXT_H_
# define LBCPP_OPTIMIZER_CONTEXT_H_

#include <lbcpp/Optimizer/OptimizerCallback.h>
#include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{
  
class OptimizerContext : public Object
{
public:
  OptimizerContext(const FunctionPtr& objectiveFunction) : objectiveFunction(objectiveFunction) {}
  OptimizerContext() {}
  
  FunctionPtr getObjectiveFunction() const
    {return objectiveFunction;}
  
  OptimizerCallbackPtr getOptimizerCallback() const
    {return optimizerCallback;}
  
  void setCallback(const OptimizerCallbackPtr callback)
    {optimizerCallback = callback;}
  
  virtual bool evaluate(const Variable& parameters, const OptimizerStatePtr& optimizerState) = 0;
  // TODO arnaud : evalaute std::vector<Variable>
  
protected:
  
  // TODO arnaud : check this method
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
  
private:
  FunctionPtr objectiveFunction;
  OptimizerCallbackPtr optimizerCallback;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;
extern ClassPtr optimizerContextClass;

  
};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_