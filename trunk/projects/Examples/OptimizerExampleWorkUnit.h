/*
 *  OptimizerExampleWorkUnit.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 1/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef OPTIMIZER_EXAMPLE_WORK_UNIT_H_
#define OPTIMIZER_EXAMPLE_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>

namespace lbcpp
{

  class OptimizerExampleWorkUnit : public WorkUnit 
  {
  public:
    
    virtual Variable run(ExecutionContext& context)
    {
      ScalarFunctionPtr square = squareFunction();
      UniformDistributionPtr apriori = new UniformDistribution(0,10);
      OptimizerPtr optimizer = uniformSampleAndPickBestOptimizer(100);
      
      Variable var = optimizer->compute(context, square, apriori);
      return var;
    }
    
  };
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_EXAMPLE_WORK_UNIT_H_