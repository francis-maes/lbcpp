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
  OptimizerContext(const FunctionPtr& objectiveFunction);
  
  OptimizerContext();
  
  void setPostEvaluationCallback(const FunctionCallbackPtr& callback);
  
  void removePostEvaluationCallback(const FunctionCallbackPtr& callback);
  
  virtual void waitUntilAllRequestsAreProcessed() const = 0;
  
  virtual bool evaluate(ExecutionContext& context, const Variable& parameters) = 0;
  
protected:
  friend class OptimizerContextClass;
  
  FunctionPtr objectiveFunction;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;
extern ClassPtr optimizerContextClass;

  
};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_
