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

namespace lbcpp
{
  
class OptimizerState : public Object
{
public:
  OptimizerState() : bestScore(DBL_MAX) {}
  
  
  // TODO arnaud : visiblility
  DistributionPtr getDistribution() const
    {return distribution;}
  
  void setDistribution(const DistributionPtr& newDistribution)
    {distribution = newDistribution;}
  
  Variable bestVariable;
  double bestScore;

protected:  
  friend class OptimizerStateClass;
  
private:
  DistributionPtr distribution;

};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;

  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_