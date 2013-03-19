/*-----------------------------------------.---------------------------------.
| Filename: SubExecutionContext.h          | Base class for Execution Contexts|
| Author  : Francis Maes                   |   that have a parent            |
| Started : 24/11/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_CONTEXT_SUB_H_
# define OIL_EXECUTION_CONTEXT_SUB_H_

# include <oil/Execution/ExecutionStack.h>
# include <oil/Execution/ExecutionContext.h>

namespace lbcpp
{

class SubExecutionContext : public ExecutionContext
{
public:
  SubExecutionContext(ExecutionContext& parentContext)
    : parent(&parentContext) {}
  SubExecutionContext() {}

  virtual juce::File getProjectDirectory() const
    {return parent ? parent->getProjectDirectory() : ExecutionContext::getProjectDirectory();}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    if (parent)
      parent->notificationCallback(notification);
    ExecutionContext::notificationCallback(notification);
  }

  virtual void waitUntilAllWorkUnitsAreDone(size_t timeOutInMilliseconds = 0)
    {if (parent) parent->waitUntilAllWorkUnitsAreDone(timeOutInMilliseconds);}

  virtual void flushCallbacks()
    {if (parent) parent->flushCallbacks();}

protected:
  friend class SubExecutionContextClass;

  ExecutionContext* parent;
};

}; /* namespace lbcpp */

#endif //!OIL_EXECUTION_CONTEXT_SUB_H_
