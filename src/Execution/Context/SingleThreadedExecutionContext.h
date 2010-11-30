/*-----------------------------------------.---------------------------------.
| Filename: SingleThreadedExecutionContext.h| Single-Threaded Execution      |
| Author  : Francis Maes                   | Context                         |
| Started : 24/11/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_
# define LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_

# include <lbcpp/Execution/ExecutionContext.h>
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class SingleThreadedExecutionContext : public ExecutionContext
{
public:
  virtual bool isMultiThread() const
    {return false;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual bool run(const WorkUnitVectorPtr& workUnits)
  {
    bool res = true;
    preExecutionCallback(workUnits);
    size_t n = workUnits->getNumWorkUnits();
    for (size_t i = 0; i < n; ++i)
      res &= ExecutionContext::run(workUnits->getWorkUnit(i));
    postExecutionCallback(workUnits, res);
    return res;
  }

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_
