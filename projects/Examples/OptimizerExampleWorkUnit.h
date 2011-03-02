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
# include <lbcpp/Optimizer/ObjectiveFunction.h>
# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>

namespace lbcpp
{

  class TestObjectiveFunction : public ObjectiveFunction 
  {
  public:
    TestObjectiveFunction() 
    {
      function = squareFunction();
      function->initialize(defaultExecutionContext(), doubleType);
      if (function->getOutputType() != doubleType) 
      {
        jassert(false); // TODO arnaud
      }
    }
    
  protected:
    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
      {return function->compute(context, input);}
    
  private:
    FunctionPtr function;
    
  };
  
  class OptimizerExampleWorkUnit : public WorkUnit 
  {
  public:
    
    virtual Variable run(ExecutionContext& context)
    {
      
      FunctionPtr f = new TestObjectiveFunction();
      UniformDistributionPtr apriori = new UniformDistribution(0,10);
      OptimizerPtr optimizer = uniformSampleAndPickBestOptimizer(100);
      
      Variable var = optimizer->compute(context, f, apriori);
      return var;
    }
    
  };
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_EXAMPLE_WORK_UNIT_H_