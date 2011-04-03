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

namespace lbcpp
{
  
class OptimizerContext : public Object
{
public:
  void setCallback(const OptimizerCallbackPtr& optimizerCallback)
  {
    callback = optimizerCallback;
  }
  
  virtual long evaluate(const Variable& parameters) = 0;
  // TODO arnaud : evalaute std::vector<Variable>
  
private:
  FunctionPtr objectiveFunction;
  OptimizerCallbackPtr callback;
};
  
typedef ReferenceCountedObjectPtr<OptimizerContext> OptimizerContextPtr;

  
};
#endif // !LBCPP_OPTIMIZER_CONTEXT_H_