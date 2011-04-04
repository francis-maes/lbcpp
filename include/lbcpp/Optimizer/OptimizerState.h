/*
 *  OptimizerState.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 4/04/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_OPTIMIZER_STATE_H_
# define LBCPP_OPTIMIZER_STATE_H_

namespace lbcpp
{
  
class OptimizerState : public Object
{
  
protected:  
  friend class OptimizerStateClass;
  
private:
  std::vector< std::pair<juce::int64, Variable> > evaluationRequests;
  std::vector< std::pair<juce::int64, double> > evaluationResults;

};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;

  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_