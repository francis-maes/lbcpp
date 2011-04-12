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
// TODO arnaud : ScopedLock in each method ? (object should be thread safe)
class OptimizerState : public Object, public FunctionCallback
{
public:
  OptimizerState();
  
  // this should be in an inherited class
  const DistributionPtr& getDistribution() const;
  
  void setDistribution(ExecutionContext& context, const DistributionPtr& newDistribution);
  
  
  /*
  ** Requests
  */
  size_t getTotalNumberOfRequests() const;

  void incTotalNumberOfRequests();
  
  size_t getTotalNumberOfEvaluations() const;
  
  /*
  ** Processeded requests
  */
  size_t getNumberOfProcessedRequests() const;
  
  const std::vector< std::pair<double, Variable> >& getProcessedRequests() const;
  
  void flushProcessedRequests();
  
  /*
  ** Best variable and score
  */
  const Variable& getBestVariable() const;
  
  double getBestScore() const;
  
  void setBestRequest(double score, const Variable& parameter);
  
  const std::vector< std::pair<double, Variable> >& getBestRequests();
  
  /*
  ** Critical Section
  */
  const CriticalSection& getLock() const;
  
  /*
   ** FunctionCallback
   */
  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output);
  
protected:  
  friend class OptimizerStateClass;
  
  CriticalSection lock;
  
  DistributionPtr distribution;
  
  size_t totalNumberOfRequests;
  size_t totalNumberOfEvaluations;
  
  std::vector< std::pair<double, Variable> > bestRequests;  // should stay sorted (decreasing by score) by construction
  
  std::vector< std::pair<double, Variable> > processedRequests;  // evaluated WUs not processed yet
};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;
  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
