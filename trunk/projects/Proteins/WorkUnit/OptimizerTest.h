/*
 *  OptimizerTest.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 2/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef OPTIMIZER_TEST_H_
#define OPTIMIZER_TEST_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/ObjectiveFunction.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>
# include "../Optimizer/ProteinGridEvoOptimizer.h"

namespace lbcpp
{
  
  class OptimizerTest : public WorkUnit 
  {
  public:
    
    virtual Variable run(ExecutionContext& context)
    {
      OptimizerPtr optimizer = new ProteinGridEvoOptimizer();
      Variable var = optimizer->compute(context, new ObjectiveFunction(), new IndependentMultiVariateDistribution(doubleType));
      return var;
      
    }
  protected:
    friend class OptimizerTestClass;
  };
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_