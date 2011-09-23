/*-----------------------------------------.---------------------------------.
| Filename: SubExecutionContext.h          | Base class for Execution Contexts|
| Author  : Francis Maes                   |   that have a parent            |
| Started : 24/11/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_SUB_H_
# define LBCPP_EXECUTION_CONTEXT_SUB_H_

# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Execution/ExecutionContext.h>

namespace lbcpp
{

class SubExecutionContext : public ExecutionContext
{
public:
  SubExecutionContext(ExecutionContext& parentContext)
    : parent(&parentContext) {}
  SubExecutionContext() {}

  virtual File getProjectDirectory() const
    {return parent ? parent->getProjectDirectory() : ExecutionContext::getProjectDirectory();}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    if (parent)
      parent->notificationCallback(notification);
    ExecutionContext::notificationCallback(notification);
  }

  virtual void flushCallbacks()
    {if (parent) parent->flushCallbacks();}

protected:
  friend class SubExecutionContextClass;

  ExecutionContext* parent;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_SUB_H_
