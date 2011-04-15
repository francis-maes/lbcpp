/*-----------------------------------------.---------------------------------.
| Filename: MultiThreadsOptimizerContext.h | OptimizerContext that uses      |
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
  
class MultiThreadsOptimizerContext : public OptimizerContext
{
public:
  MultiThreadsOptimizerContext(const FunctionPtr& objectiveFunction, size_t numThreads = (size_t)juce::SystemStats::getNumCpus())
    : OptimizerContext(objectiveFunction), numThreads(numThreads) {}
  MultiThreadsOptimizerContext() {}
    
  virtual void waitUntilAllRequestsAreProcessed() const 
  {
    multiThreadedContext->waitUntilAllWorkUnitsAreDone();
  }
  
  virtual bool evaluate(ExecutionContext& context, const Variable& parameters) 
  { 
    if (!multiThreadedContext.exists()) {
      multiThreadedContext = multiThreadedExecutionContext(numThreads);
      multiThreadedContext->setCallbacks(context.getCallbacks());
    }
    WorkUnitPtr wu = new FunctionWorkUnit(objectiveFunction, parameters);
    multiThreadedContext->pushWorkUnit(wu);
    // callback is done in function evaluation !
    return true;
  }
  
protected:  
  friend class MultiThreadsOptimizerContextClass;
  
private:
  size_t numThreads;
  ExecutionContextPtr multiThreadedContext;
};
  
};
#endif // !LBCPP_MULTI_THEADS_OPTIMIZER_CONTEXT_H_
