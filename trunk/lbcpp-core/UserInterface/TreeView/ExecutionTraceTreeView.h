/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.h       | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 28/11/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_TREE_VIEW_EXECUTION_TRACE_H_
# define LBCPP_USER_INTERFACE_TREE_VIEW_EXECUTION_TRACE_H_

# include "GenericTreeView.h"
# include <lbcpp/Execution/Notification.h>
# include <lbcpp/Execution/ExecutionTrace.h>

namespace lbcpp
{
  
class ExecutionTraceTreeViewItem : public GenericTreeViewItem
{
public:
  ExecutionTraceTreeViewItem(GenericTreeView* owner, const ExecutionTraceItemPtr& trace);

  virtual int getMaximumColumnWidth(size_t columnNumber) const;
  virtual void paintColumn(Graphics& g, size_t columnNumber, ObjectPtr data, int x, int y, int width, int height) const;
  virtual ObjectPtr getTargetObject(ExecutionContext& context) const;

  const ExecutionTraceItemPtr& getTrace() const
    {return object.staticCast<ExecutionTraceItem>();}
  
  ExecutionTraceNodePtr getTraceNode() const
    {return object.dynamicCast<ExecutionTraceNode>();}
};

class ExecutionTraceTreeView : public GenericTreeView, public ExecutionCallback
{
public:
  ExecutionTraceTreeView(ExecutionTracePtr trace, const string& name, ExecutionContextPtr context = ExecutionContextPtr());
  virtual ~ExecutionTraceTreeView();

  ExecutionTraceTreeViewItem* getNodeFromStack(const ExecutionStackPtr& stack) const;

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
  virtual size_t getNumDataColumns();
  virtual std::vector<ObjectPtr> getObjectData(const ObjectPtr& object);
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_EXECUTION_TRACE_H_
