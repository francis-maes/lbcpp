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
  OptimizerState() : totalNumberOfRequests(0), totalNumberOfEvaluations(0), bestScore(DBL_MAX) {}
  
  // OptimizerState
  const CriticalSection& getLock() const
    {return lock;}
  
  const DistributionPtr& getDistribution() const
    {return distribution;}
  void setDistribution(const DistributionPtr& newDistribution)
    {distribution = newDistribution;}
  
  size_t getTotalNumberOfRequests() const
    {return totalNumberOfRequests;}
  void incTotalNumberOfRequests()
    {totalNumberOfRequests++;}
  
  size_t getTotalNumberOfEvaluations() const
    {return totalNumberOfEvaluations;}
  void incTotalNumberOfEvaluations()
    {totalNumberOfEvaluations++;}
  
  const Variable& getBestVariable() const
    {return bestVariable;}
  void setBestVariable(const Variable& variable)
    {bestVariable = variable;}
  
  double getBestScore() const
    {return bestScore;}
  void setBestScore(double score)
    {bestScore = score;}
  
  size_t getNumberOfUnprocessedEvaluations() const
    {return unprocessedEvaluations.size();}
  const std::vector< std::pair<double, Variable> >& getUnprocessedEvaluations() const
    {return unprocessedEvaluations;}
  void clearUnprocessedEvaluations()
    {unprocessedEvaluations.clear();}

  
  // FunctionCallback
  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output) 
  {
    ScopedLock _(lock);
    unprocessedEvaluations.push_back(std::make_pair(OptimizerContext::getDoubleFromOutput(output),inputs[0]));
    incTotalNumberOfEvaluations();
  }

protected:  
  friend class OptimizerStateClass;
  
private:
  CriticalSection lock;
  
  DistributionPtr distribution;
  
  size_t totalNumberOfRequests;
  size_t totalNumberOfEvaluations;
  
  Variable bestVariable;
  double bestScore;
  
  std::vector< std::pair<double, Variable> > unprocessedEvaluations;  // evaluated WUs not processed yet
};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;

  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
