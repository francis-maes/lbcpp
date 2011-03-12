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
# include <lbcpp/Optimizer/GridOptimizer.h>
# include "../Optimizer/ProteinGridEvoOptimizer.h"
# include "../../../src/Optimizer/Optimizer/Grid/GridEvoOptimizer.h"
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../Predictor/ProteinPredictorParameters.h"
# include "../../../src/Distribution/Builder/GaussianDistributionBuilder.h"
# include "../../../src/Distribution/Builder/BernoulliDistributionBuilder.h"

namespace lbcpp
{
  
  class OptimizerTest : public WorkUnit 
  {
  public:
    
    virtual Variable run(ExecutionContext& context)
    {
      
      // TODO arnaud : add getBuilder() in Distribution.h
      IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
      distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(1,0));
      distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(3,0));
      distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,0));
      distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(3,0));
      distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,0));
      distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(3,0));
      distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(2,0));
      distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(3,0));
      distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,0));
      distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,0));
      distributions->setSubDistribution(10, new BernoulliDistribution(1));
      distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(15,0));
      distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(15,0));
      distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(50,0));
      
      //IndependentMultiVariateDistributionBuilderPtr distributionsBuilder = distributions->createBuilder();
      
      
      //GridEvoOptimizerPtr optimizer = new GridEvoOptimizer();
      //return optimizer->optimize(context, new ProteinGridEvoOptimizerState(distributions), new ProteinGetVariableFromTraceFunction(), new ProteinGetScoreFromTraceFunction());
      
      
      ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState(distributions);
      WorkUnitPtr wu = state->generateSampleWU(context);
      wu->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("wu.xml")));
      
      
      //ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState(distributions);
      //state->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("test.xml"))); // --->  Could not find type ProteinGridEvoOptimizerState (in TypeManager::getType()) 
      
      //ProteinGridEvoOptimizerStatePtr state2 = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("test.xml"))).staticCast<ProteinGridEvoOptimizerState>();

      
      /*
      ExecutionTracePtr trace = Object::createFromFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/1299675529047.trace"))).staticCast<ExecutionTrace>();
      FunctionPtr f1 = new ProteinGetScoreFromTraceFunction();
      std::cout << f1->compute(context, trace).toString() << std::endl;
      
      FunctionPtr f2 = new ProteinGetVariableFromTraceFunction();
      std::cout << f2->compute(context, trace).toString() << std::endl;
      */
          
            
      
      return Variable();
      
    }
  protected:
    friend class OptimizerTestClass;
  };
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_