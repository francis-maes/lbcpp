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
  
class ExecutionTraceTreeView;
class ExecutionTraceTreeViewItem : public GenericTreeViewItem
{
public:
  ExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace);

  static ExecutionTraceTreeViewItem* create(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace);

  enum
  {
    minWidthToDisplayTimes = 300,
    minWidthToDisplayProgression = 150,
    progressionColumnWidth = 150,
    labelX = 23,
    timeColumnWidth = 100,
  };
 
  void paintProgression(juce::Graphics& g, ProgressionStatePtr progression, int x, int width, int height);
  void paintIcon(juce::Graphics& g, int width, int height);
  void paintIconTextAndProgression(juce::Graphics& g, int width, int height);

  virtual void paintItem(juce::Graphics& g, int width, int height);
  
  virtual int getItemHeight() const
    {return 20 * numLines;}

  const ExecutionTraceItemPtr& getTrace() const
    {return object.staticCast<ExecutionTraceItem>();}

protected:
  int numLines;
};

class ExecutionTraceTreeViewNode : public ExecutionTraceTreeViewItem
{
public:
  ExecutionTraceTreeViewNode(ExecutionTraceTreeView* owner, const ExecutionTraceNodePtr& trace);

  const ExecutionTraceNodePtr& getTraceNode() const
    {return object.staticCast<ExecutionTraceNode>();}

  virtual void itemOpennessChanged(bool isNowOpen);

  virtual ObjectPtr getTargetObject(ExecutionContext& context) const;
};

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

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_EXECUTION_TRACE_H_
