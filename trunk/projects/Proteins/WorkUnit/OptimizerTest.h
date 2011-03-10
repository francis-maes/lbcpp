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
# include "../../../src/Optimizer/Optimizer/GridEvoOptimizer.h"
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../Predictor/ProteinPredictorParameters.h"

namespace lbcpp
{
  
  class OptimizerTest : public WorkUnit 
  {
  public:
    
    virtual Variable run(ExecutionContext& context)
    {
      //ExecutionTracePtr trace = Object::createFromFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/1299675529047.trace"))).staticCast<ExecutionTrace>();
      //ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/1299675529047.trace")));
      ProteinGridEvoOptimizerPtr optimizer = new ProteinGridEvoOptimizer();
      return optimizer->optimize(context, new Function(), Variable());
      
      //std::cout << optimizer->getVariableFromTrace(trace) << std::endl;
      
      
      /*XmlExporter exporter(context);                      
      distributions->saveToXml(exporter);
      exporter.saveToFile(File(T("/Users/arnaudschoofs/Proteins/traces/test.xml")));*/
      
      /*distributions->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/test.xml"))); 
      IndependentMultiVariateDistributionPtr test = Object::createFromFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/test.xml"))).staticCast<IndependentMultiVariateDistribution>();*/  

      
      
      //Variable var = optimizer->compute(context, new Function(), distributions);
      //return Variable();
      
    }
  protected:
    friend class OptimizerTestClass;
  };
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_