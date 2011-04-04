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

namespace lbcpp
{

class OptimizerCallback : public Object
{
public:
  virtual void evaluationFinished(juce::int64 identifier, double score) = 0;
};

typedef ReferenceCountedObjectPtr<OptimizerCallback> OptimizerCallbackPtr;

  
};
#endif // !LBCPP_OPTIMIZER_CALLBACK_H_