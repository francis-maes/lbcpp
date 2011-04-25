/*-----------------------------------------.---------------------------------.
| Filename: MultiThreadedOptimizerContext.h| OptimizerContext that uses      |
| Author  : Arnaud Schoofs                 | multi-threads to evaluate the   |
| Started : 03/04/2011                     | requests from the Optimizer     |
`------------------------------------------/ (asynchronous)                  |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_
# define LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>

namespace lbcpp
{  
  
class MultiThreadedOptimizerContext : public OptimizerContext
{
public:
  MultiThreadedOptimizerContext(const FunctionPtr& objectiveFunction)
    : OptimizerContext(objectiveFunction) {}
  MultiThreadedOptimizerContext() {}
    
  virtual void waitUntilAllRequestsAreProcessed(ExecutionContext& context) const 
    {context.waitUntilAllWorkUnitsAreDone();}
  
  virtual bool evaluate(ExecutionContext& context, const Variable& parameters) 
  { 
    context.pushWorkUnit(new FunctionWorkUnit(objectiveFunction, parameters));
    return true;
  }

  virtual bool evaluate(ExecutionContext& context, const std::vector<Variable>& parametersVector)
  {
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating ") + String((int)parametersVector.size()) + T(" parameters"), parametersVector.size());
    for (size_t i = 0; i < parametersVector.size(); ++i)
      workUnit->setWorkUnit(i, new FunctionWorkUnit(objectiveFunction, parametersVector[i]));
    workUnit->setProgressionUnit(T("Parameters"));
    workUnit->setPushChildrenIntoStackFlag(false);
    context.run(workUnit); // FIXME: should be pushWorkUnit
    return true;
  }
};
  
};
#endif // !LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_
