/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.h       | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 28/11/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
# define LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_

#include "../../../explorer/Utilities/SimpleTreeViewItem.h" // FIXME! move utilities inside lbcpp-core
#include "../../../explorer/Utilities/VariableSelector.h"
#include "../../../explorer/Utilities/ComponentWithPreferedSize.h"

using juce::Component;
using juce::DocumentWindow;

using juce::Image;
using juce::Colours;
using juce::Justification;

using juce::TreeView;
using juce::TreeViewItem;

namespace lbcpp
{

class ExecutionTraceTreeView;
class ExecutionTraceTreeViewItem : public SimpleTreeViewItem
{
public:
  ExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace);

  enum
  {
    minWidthToDisplayTimes = 300,
    minWidthToDisplayProgression = 150,
    progressionColumnWidth = 150,
    labelX = 23,
    timeColumnWidth = 100,
  };
 
  void paintProgression(Graphics& g, WorkUnitExecutionTraceItemPtr workUnitTrace, int x, int width, int height);
  void paintIcon(Graphics& g, int width, int height);
  void paintIconTextAndProgression(Graphics& g, int width, int height);

  virtual void paintItem(Graphics& g, int width, int height);

  static String formatTime(double timeInSeconds);
  
  virtual int getItemHeight() const
    {return 20 * numLines;}

  const ExecutionTraceItemPtr& getTrace() const
    {return trace;}

  virtual void itemSelectionChanged(bool isNowSelected);
  
protected:
  ExecutionTraceTreeView* owner;
  ExecutionTraceItemPtr trace;
  int numLines;
};

class WorkUnitExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  WorkUnitExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const WorkUnitExecutionTraceItemPtr& trace)
    : ExecutionTraceTreeViewItem(owner, trace)
  {
    CompositeWorkUnitPtr compositeWorkUnit = trace->getWorkUnit().dynamicCast<CompositeWorkUnit>();
    setOpen(!compositeWorkUnit || compositeWorkUnit->getNumWorkUnits() <= 10);
  }

  virtual bool mightContainSubItems()
    {return getNumSubItems() > 0;}

  const WorkUnitExecutionTraceItemPtr& getTrace() const
    {return trace.staticCast<WorkUnitExecutionTraceItem>();}

  const WorkUnitPtr& getWorkUnit() const
    {return getTrace()->getWorkUnit();}
};

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

  WorkUnitExecutionTraceTreeViewItem* getItemFromStack(const ExecutionStackPtr& stack) const;

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
