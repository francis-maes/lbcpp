/*-----------------------------------------.---------------------------------.
| Filename: OptimizerExampleWorkUnit.h     | Example WorkUnit for Optimizer  |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 01/03/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OPTIMIZER_EXAMPLE_WORK_UNIT_H_
#define OPTIMIZER_EXAMPLE_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

class OptimizerExampleWorkUnit : public WorkUnit 
{
public:
  
  virtual Variable run(ExecutionContext& context)
  {
    size_t numIterations = 40;
    size_t populationSize = 100;
    size_t numBests = 30;
    
    FunctionPtr f = squareFunction();
    SamplerPtr sampler = gaussianSampler(0.0, 5.0);
    
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, 0.0);
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 10);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);
    
    return optimizer->compute(context, optimizerContext, optimizerState);
  }
};
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_EXAMPLE_WORK_UNIT_H_