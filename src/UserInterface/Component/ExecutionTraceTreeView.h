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
 
class ExecutionTraceTreeView : public TreeView, public DelayToUserInterfaceExecutionCallback, public VariableSelector, public ComponentWithPreferedSize
{
public:
  ExecutionTraceTreeView(ExecutionTracePtr trace);
  virtual ~ExecutionTraceTreeView();

  ExecutionTraceTreeViewNode* getItemFromStack(const ExecutionStackPtr& stack) const;

  double getInitialTime() const
    {return initialTime;}

  virtual void timerCallback();

  void invalidateTree();
  void invalidateSelection();

  virtual int getDefaultWidth() const;

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionTracePtr trace;
  double initialTime;
  bool isSelectionUpToDate;
  bool isTreeUpToDate;

  ExecutionCallbackPtr createTreeBuilderCallback();
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
