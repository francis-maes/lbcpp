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
# include "OptimizerTestBed.h"

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

    SamplerPtr sampler = gaussianSampler(0.0, 5.0);
    
    OptimizerPtr optimizer = edaOptimizer(sampler, numIterations, populationSize, numBests, StoppingCriterionPtr(), 0.0);
    return optimizer->compute(context, squareFunction());
  }
};

class OptimizerTestBedWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    static const size_t N = 10;

    DenseDoubleVectorPtr xopt = new DenseDoubleVector(N, 1.0);
    double fopt = 51.0;
    FunctionPtr objective = new SphereFunction(xopt, fopt);

    //SamplerPtr sampler = independentDoubleVectorSampler(N, gaussianSampler(0.0, 5.0));
    //OptimizerPtr optimizer = edaOptimizer(sampler, 100, 50, 10, StoppingCriterionPtr(), 0.0);

    OptimizerPtr optimizer = cmaesOptimizer(new DenseDoubleVector(N, 0.0), 100);
    return optimizer->compute(context, objective);
  }
};

};/* namespace lbcpp */

#endif // !OPTIMIZER_EXAMPLE_WORK_UNIT_H_
