/*-----------------------------------------.---------------------------------.
| Filename: ThreadOwnedExecutionContext.h  | Thread Owned Execution Context  |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_THREAD_OWNED_H_
# define LBCPP_EXECUTION_CONTEXT_THREAD_OWNED_H_

# include "DecoratorExecutionContext.h"

namespace lbcpp
{

class ThreadOwnedExecutionContext : public DecoratorExecutionContext, public Thread
{
public:
  ThreadOwnedExecutionContext(ExecutionContextPtr context, WorkUnitPtr workUnit)
    : DecoratorExecutionContext(context), Thread(workUnit->getName()), workUnit(workUnit) {}
  ThreadOwnedExecutionContext() : Thread(T("Empty")) {}

  virtual bool isCanceled() const
    {return threadShouldExit();}

  virtual void run()
  {
    juce::SystemStats::initialiseStats();
    DecoratorExecutionContext::run(workUnit);
  }

  WorkUnitPtr getWorkUnit() const
    {return workUnit;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ThreadOwnedExecutionContextClass;

  WorkUnitPtr workUnit;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_THREAD_OWNED_H_
