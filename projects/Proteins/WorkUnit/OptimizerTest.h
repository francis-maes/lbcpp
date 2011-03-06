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
# include <lbcpp/Distribution/ContinuousDistribution.h>
# include "../Optimizer/ProteinGridEvoOptimizer.h"
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../Predictor/ProteinPredictorParameters.h"

namespace lbcpp
{
  
  class OptimizerTest : public WorkUnit 
  {
  public:
    
    virtual Variable run(ExecutionContext& context)
    {
      OptimizerPtr optimizer = new ProteinGridEvoOptimizer();
      
      IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
      distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
      distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(5,1));
      distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(5,1));
      
      
      Variable var = optimizer->compute(context, new Function(), distributions);
      return var;
      
    }
  protected:
    friend class OptimizerTestClass;
  };
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_