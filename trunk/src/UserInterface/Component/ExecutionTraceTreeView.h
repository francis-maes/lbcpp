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

using juce::Component;
using juce::DocumentWindow;

using juce::Image;
using juce::Colours;
using juce::Justification;

using juce::TreeView;
using juce::TreeViewItem;

namespace lbcpp
{

class ExecutionTraceTreeViewItem : public SimpleTreeViewItem
{
public:
  ExecutionTraceTreeViewItem(const String& name, const String& iconToUse, bool mightContainSubItems);

  enum
  {
    minWidthToDisplayTimes = 300,
    minWidthToDisplayProgression = 150,
    progressionColumnWidth = 150,
    labelX = 23,
    timeColumnWidth = 100,
  };
 
  void paintProgression(Graphics& g, int x, int width, int height);
  void paintIcon(Graphics& g, int width, int height);
  void paintIconTextAndProgression(Graphics& g, int width, int height);

  virtual void paintItem(Graphics& g, int width, int height);

  void setProgression(double progression, double progressionTotal, const String& unit);

  static String formatTime(double timeInSeconds);
  
  void setStartTime(double time);
  void setEndTime(double time);

protected:
  double creationTime;

  String absoluteTime;
  String relativeTime;

  bool hasProgression;
  double progression;
  String progressionString;
};

class NodeExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  NodeExecutionTraceTreeViewItem(const WorkUnitPtr& workUnit, const String& iconToUse, bool open = true)
    : ExecutionTraceTreeViewItem(workUnit ? workUnit->getName() : String::empty, iconToUse, false), workUnit(workUnit)
    {setOpen(open);}

  const WorkUnitPtr& getWorkUnit() const
    {return workUnit;}

  virtual bool mightContainSubItems()
    {return getNumSubItems() > 0;}

protected:
  WorkUnitPtr workUnit;
};

class WorkUnitExecutionTraceTreeViewItem : public NodeExecutionTraceTreeViewItem
{
public:
  WorkUnitExecutionTraceTreeViewItem(const WorkUnitPtr& workUnit)
    : NodeExecutionTraceTreeViewItem(workUnit, T("WorkUnit-32.png")) {}
};

class CompositeWorkUnitExecutionTraceTreeViewItem : public NodeExecutionTraceTreeViewItem
{
public:
  CompositeWorkUnitExecutionTraceTreeViewItem(const CompositeWorkUnitPtr& workUnits)
    : NodeExecutionTraceTreeViewItem(workUnits, T("WorkUnit-32.png"), workUnits->getNumWorkUnits() < 10),
      progressionUnit(workUnits->getProgressionUnit()), numWorkUnits(workUnits->getNumWorkUnits()), numWorkUnitsDone(0) {}

  virtual void paintItem(Graphics& g, int width, int height)
  {
    setProgression((double)numWorkUnitsDone, (double)numWorkUnits, progressionUnit);
    ExecutionTraceTreeViewItem::paintItem(g, width, height);
  }

  void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
    {++numWorkUnitsDone;}

private:
  String progressionUnit;
  size_t numWorkUnits;
  size_t numWorkUnitsDone;
};

class MessageExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  MessageExecutionTraceTreeViewItem(const String& what, const String& where = String::empty, const String& iconToUse = String::empty)
    : ExecutionTraceTreeViewItem(what + (where.isNotEmpty() ? T(" (") + where + T(")") : String::empty), iconToUse, false)
  {
    String str = getUniqueName();
    numLines = 1;
    for (int i = 0; i < str.length() - 1; ++i)
      if (str[i] == '\n')
        ++numLines;
  }

  virtual int getItemHeight() const
    {return 20 * numLines;}

private:
  size_t numLines;
};

class WarningExecutionTraceTreeViewItem : public MessageExecutionTraceTreeViewItem
{
public:
  WarningExecutionTraceTreeViewItem(const String& what, const String& where = String::empty)
    : MessageExecutionTraceTreeViewItem(what, where, T("Warning-32.png")) {}
};

class ErrorExecutionTraceTreeViewItem : public MessageExecutionTraceTreeViewItem
{
public:
  ErrorExecutionTraceTreeViewItem(const String& what, const String& where = String::empty)
    : MessageExecutionTraceTreeViewItem(what, where, T("Error-32.png")) {}
};

class InformationExecutionTraceTreeViewItem : public MessageExecutionTraceTreeViewItem
{
public:
  InformationExecutionTraceTreeViewItem(const String& what, const String& where = String::empty)
    : MessageExecutionTraceTreeViewItem(what, where, T("Information-32.png")) {}
};

class ProgressExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  ProgressExecutionTraceTreeViewItem()
    : ExecutionTraceTreeViewItem(T("progression"), T("Progress-32.png"), false) {}
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
 
class ExecutionTraceTreeView : public TreeView, public DelayToUserInterfaceExecutionCallback
{
public:
  ExecutionTraceTreeView(ExecutionTracePtr trace);
  virtual ~ExecutionTraceTreeView();

  NodeExecutionTraceTreeViewItem* getItemFromStack(const ExecutionStackPtr& stack) const;

  double getInitialTime() const
    {return initialTime;}

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionTracePtr trace;
  double initialTime;

  ExecutionCallbackPtr createTreeBuilderCallback();
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
