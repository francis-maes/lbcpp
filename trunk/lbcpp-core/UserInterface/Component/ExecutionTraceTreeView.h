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

class ExecutionTraceTreeView : public GenericTreeView, public ExecutionCallback
{
public:
  ExecutionTraceTreeView(ExecutionTracePtr trace, const string& name, ExecutionContextPtr context = ExecutionContextPtr());
  virtual ~ExecutionTraceTreeView();

  ExecutionTraceTreeViewNode* getNodeFromStack(const ExecutionStackPtr& stack) const;

  virtual void timerCallback();

  const ExecutionTracePtr& getTrace() const
    {return object.staticCast<ExecutionTrace>();}

  virtual juce::Component* createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const string& name);

  virtual void notificationCallback(const NotificationPtr& notification)
    {notificationQueue->push(notification);}

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionTracePtr trace;
  ExecutionContextPtr context;

  NotificationQueuePtr notificationQueue;
  ExecutionCallbackPtr targetCallback;

  ExecutionCallbackPtr createTreeBuilderCallback();

  virtual GenericTreeViewItem* createItem(const ObjectPtr& object, const string& name);
  virtual bool mightHaveSubObjects(const ObjectPtr& object);
  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object);
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
