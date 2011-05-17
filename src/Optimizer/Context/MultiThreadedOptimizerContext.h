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

namespace lbcpp
{  
  
class MultiThreadedOptimizerContext : public OptimizerContext
{
public:
  MultiThreadedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction, size_t timeToSleep = 100)
    : OptimizerContext(context, objectiveFunction, validationFunction, timeToSleep) 
    {numEvaluationInProgress = 0;}
  MultiThreadedOptimizerContext() 
  {
    numEvaluationInProgress = 0;
    timeToSleep = 100;
  }
  
  virtual bool areAllRequestsProcessed() const
    {return numEvaluationInProgress == 0;}
  
  virtual bool evaluate(const Variable& parameters) 
  {
    juce::atomicIncrement(numEvaluationInProgress);
    context.pushWorkUnit(new FunctionWorkUnit(objectiveFunction, parameters), &numEvaluationInProgress, false); // TODO arnaud verbose ?
    return true;
  }
  
protected:
  friend class MultiThreadedOptimizerContextClass;
  
private:
  int numEvaluationInProgress;
};
  
};
#endif // !LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_
