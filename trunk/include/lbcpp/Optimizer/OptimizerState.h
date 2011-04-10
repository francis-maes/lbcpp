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

#include <lbcpp/Distribution/Distribution.h>
#include <lbcpp/Core/Function.h>
#include <lbcpp/Optimizer/OptimizerContext.h>

namespace lbcpp
{
  
class OptimizerState : public Object, public FunctionCallback
{
public:
  OptimizerState() : bestScore(DBL_MAX) {}
  
  
  // FunctionCallback
  //virtual void functionCalled(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs) {}
  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output) 
  {
    ScopedLock _(lock);
    currentEvaluatedWUs.push_back(std::make_pair(OptimizerContext::getDoubleFromOutput(output),inputs[0]));
    totalNumberEvaluatedWUs++;
  } // TODO arnaud : check size input elsewher


  void setDistribution(const DistributionPtr& newDistribution)
    {distribution = newDistribution;}
  
protected:  
  friend class OptimizerStateClass;
  friend class UniformSampleAndPickBestOptimizer; // TODO arnaud : change this or add accessor method
  friend class EDAOptimizer;
  
  
private:
  
  CriticalSection lock;
  
  DistributionPtr distribution;
  
  size_t totalNumberGeneratedWUs;
  size_t totalNumberEvaluatedWUs;
  
  Variable bestVariable;
  double bestScore;
  
  std::vector< std::pair<double, Variable> > currentEvaluatedWUs;  // evaluated WUs not processed yet
  

};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;

  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
