/*-----------------------------------------.---------------------------------.
| Filename: DistributedOptimizerContext.h  | OptimizerContext used to        |
| Author  : Julien Becker                  | distribute work on NIC3, BOINC, |
| Started : 18/08/2011 10:20               | ... (asynchronous)              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
# define LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_

# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class DistributedOptimizerContext : public OptimizerContext
{
public:
  DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction)
    : OptimizerContext(context, objectiveFunction), numFinishedEvaluations(0) {}

  virtual bool isSynchroneous() const
    {return false;}
  
  virtual bool areAllRequestsProcessed() const
    {return numFinishedEvaluations == results.size();}
  
  virtual bool evaluate(const Variable& parameters);

  void setResult(size_t resultIndex, const Variable& result)
  {
    jassert(results.size() > resultIndex);
    results[resultIndex] = result;
    juce::atomicIncrement((int&)numFinishedEvaluations);
  }

protected:  
  friend class DistributedOptimizerContextClass;

  CriticalSection lock;
  std::vector<Variable> results;
  size_t numFinishedEvaluations;

  DistributedOptimizerContext()
    : numFinishedEvaluations(0) {}
};

typedef ReferenceCountedObjectPtr<DistributedOptimizerContext> DistributedOptimizerContextPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
