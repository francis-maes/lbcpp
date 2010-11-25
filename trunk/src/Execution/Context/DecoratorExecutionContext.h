/*-----------------------------------------.---------------------------------.
| Filename: DecoratorExecutionContext.h    | Base class for Execution Context|
| Author  : Francis Maes                   |   decorators                    |
| Started : 24/11/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_DECORATOR_H_
# define LBCPP_EXECUTION_CONTEXT_DECORATOR_H_

# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Execution/ExecutionContext.h>

namespace lbcpp
{

class DecoratorExecutionContext : public ExecutionContext
{
public:
  DecoratorExecutionContext(ExecutionContextPtr decorated)
    : decorated(decorated)
  {
    if (decorated)
    {
      appendCallback(decorated);
      stack = ExecutionStackPtr(new ExecutionStack(*decorated->getStack()));
    }
  }

  DecoratorExecutionContext() {}

  virtual bool isMultiThread() const
    {return decorated->isMultiThread();}

  virtual bool isCanceled() const
    {return decorated && decorated->isCanceled();}

  virtual bool isPaused() const
    {return decorated && decorated->isPaused();}

  virtual bool run(const WorkUnitPtr& workUnit)
    {return decorated && decorated->run(workUnit);}

  virtual bool run(const std::vector<WorkUnitPtr>& workUnits)
    {return decorated && decorated->run(workUnits);}

protected:
  friend class DecoratorExecutionContextClass;

  ExecutionContextPtr decorated;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_DECORATOR_H_
