/*-----------------------------------------.---------------------------------.
| Filename: DistributedOptimizerContext.h  | OptimizerContext used to        |
| Author  : Julien Becker                  | distribute work on NIC3, BOINC, |
| Started : 18/08/2011 10:20               | ... (asynchronous)              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "DistributedOptimizerContext.h"
#include <lbcpp/Execution/ExecutionContextCallback.h>

namespace lbcpp
{

class OptimizerExecutionContextCallback : public ExecutionContextCallback
{
public:
  OptimizerExecutionContextCallback(const DistributedOptimizerContextPtr& optimizerContext, size_t resultIndex)
    : optimizerContext(optimizerContext), resultIndex(resultIndex) {}
  
  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
    {optimizerContext->setResult(resultIndex, result);}
  
protected:
  DistributedOptimizerContextPtr optimizerContext;
  size_t resultIndex;
};
  
}; /* namespace lbcpp */

using namespace lbcpp;

bool DistributedOptimizerContext::evaluate(const Variable& parameters)
{
  ScopedLock _(lock);
  WorkUnitPtr workUnit = new FunctionWorkUnit(objectiveFunction, parameters);

  const size_t resultIndex = results.size();
  results.push_back(Variable());
  context.pushWorkUnit(workUnit, new OptimizerExecutionContextCallback(refCountedPointerFromThis(this), resultIndex));

  return true;
}
