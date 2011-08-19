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
#if 0
class DistributedOptimizerContext : public OptimizerContext, public ExecutionContextCallback
{
public:
  DistributedOptimizerContext(ExecutionContext& context, const FunctionPtr& objectiveFunction)
    : OptimizerContext(context, objectiveFunction), numFinishedEvaluations(0) {}

  virtual bool isSynchroneous() const
    {return false;}

  virtual bool areAllRequestsProcessed() const
    {return numFinishedEvaluations == results.size();}

  virtual bool evaluate(const Variable& parameters)
  {
    ScopedLock _(lock);
    WorkUnitPtr workUnit = new FunctionWorkUnit(objectiveFunction, parameters);

    workUnitIndices[workUnit] = results.size();
    results.push_back(Variable());
    context.pushWorkUnit(workUnit, refCountedPointerFromThis(this));

    return true;
  }

  /* ExecutionContextCallback */
  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    jassert(workUnitIndices.count(workUnit) == 1);
    results[workUnitIndices[workUnit]] = result;
    juce::atomicIncrement((int&)numFinishedEvaluations);
  }

protected:
  friend class DistributedOptimizerContextClass;

  CriticalSection lock;
  std::map<WorkUnitPtr, size_t> workUnitIndices;
  std::vector<Variable> results;
  size_t numFinishedEvaluations;

  DistributedOptimizerContext()
    : numFinishedEvaluations(0) {}
};

typedef ReferenceCountedObjectPtr<DistributedOptimizerContext> DistributedOptimizerContextPtr;
#endif //!0
}; /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTED_OPTIMIZER_CONTEXT_H_
