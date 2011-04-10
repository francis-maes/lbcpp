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

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{
  
class OptimizerContext : public Object
{
public:
  OptimizerContext(const FunctionPtr& objectiveFunction) : objectiveFunction(objectiveFunction) 
    {jassert(objectiveFunction->getNumRequiredInputs() == 1);}
  OptimizerContext() {}
  
  void setPostEvaluationCallback(const FunctionCallbackPtr& callback)
    {objectiveFunction->addPostCallback(callback);}
  
  void removePostEvaluationCallback(const FunctionCallbackPtr& callback)
    {objectiveFunction->removePostCallback(callback);}
  
  virtual void waitAllEvaluationsFinished() const = 0;
  
  virtual bool evaluate(const Variable& parameters) = 0;
  
  static double getDoubleFromOutput(const Variable& variable)
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
  
protected:
  friend class OptimizerContextClass;
  
  FunctionPtr objectiveFunction;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;
extern ClassPtr optimizerContextClass;

  
};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_
