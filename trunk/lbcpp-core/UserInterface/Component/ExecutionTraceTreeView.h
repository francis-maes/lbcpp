/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.h       | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 28/11/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
# define LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_

# include "ExecutionTraceTreeViewItem.h"
# include <lbcpp/Execution/Notification.h>

namespace lbcpp
{

class DelayToUserInterfaceExecutionCallback : public ExecutionCallback, public juce::Timer
{
public:
  DelayToUserInterfaceExecutionCallback(ExecutionCallbackPtr target = ExecutionCallbackPtr())
    : notifications(new NotificationQueue()), target(target)
    {startTimer(100);}

  virtual void notificationCallback(const NotificationPtr& notification)
    {notifications->push(notification);}

  virtual void timerCallback()
    {notifications->flush(target);}

protected:
  NotificationQueuePtr notifications;
  ExecutionCallbackPtr target;
};
 
class ExecutionTraceTreeView : public GenericTreeView, public DelayToUserInterfaceExecutionCallback
{
public:
  ExecutionTraceTreeView(ExecutionTracePtr trace, const string& name, ExecutionContextPtr context = ExecutionContextPtr());
  virtual ~ExecutionTraceTreeView();

  ExecutionTraceTreeViewNode* getNodeFromStack(const ExecutionStackPtr& stack) const;

  virtual void timerCallback();

  void invalidateTree();
  void invalidateSelection();

  const ExecutionTracePtr& getTrace() const
    {return object.staticCast<ExecutionTrace>();}

  virtual juce::Component* createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const string& name);

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionTracePtr trace;
  ExecutionContextPtr context;
  bool isSelectionUpToDate;
  bool isTreeUpToDate;

  ExecutionCallbackPtr createTreeBuilderCallback();
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
