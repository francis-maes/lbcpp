/*
 *  OptimizerCallback.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 3/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_OPTIMIZER_CALLBACK_H_
# define LBCPP_OPTIMIZER_CALLBACK_H_

#include <lbcpp/Optimizer/OptimizerState.h>

namespace lbcpp
{

class OptimizerCallback
{
public:
  virtual ~OptimizerCallback() {}
  
  virtual void evaluationFinished(const Variable&, double score, const OptimizerStatePtr& optimizerState) = 0;
};

typedef OptimizerCallback* OptimizerCallbackPtr;

  
};
#endif // !LBCPP_OPTIMIZER_CALLBACK_H_