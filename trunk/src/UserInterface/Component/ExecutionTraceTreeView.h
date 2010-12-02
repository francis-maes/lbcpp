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
  ExecutionTraceTreeViewItem(const String& name, const String& iconToUse, bool mightContainSubItems)
    : SimpleTreeViewItem(name, iconToUse, mightContainSubItems), hasProgression(false), progression(0.0)
  {
  }

  enum
  {
    minWidthToDisplayTimes = 300,
    minWidthToDisplayProgression = 150,
    progressionColumnWidth = 150,
    labelX = 23,
    timeColumnWidth = 100,
  };
 
  void paintProgression(Graphics& g, int x, int width, int height)
  {
    juce::GradientBrush brush(Colour(200, 220, 240), (float)x, -(float)width / 3.f, Colours::white, (float)width, (float)width, true);

    g.setBrush(&brush);
    if (progression > 0.0)
    {
      jassert(progression <= 1.0);
      g.fillRect(x, 0, (int)(width * progression + 0.5), height);
    }
    else if (progression < 0.0)
    {
      double p = (-progression / 1000.0);
      p -= (int)p;
      jassert(p >= 0.0 && p <= 1.0);
      g.fillRect(x + (int)(width * p + 0.5) - 3, 0, 6, height);
    }
    g.setBrush(NULL);

    g.setColour(Colours::black);
    g.setFont(10);
    g.drawText(progressionString, x, 0, width, height, Justification::centred, true);

    g.setColour(Colour(180, 180, 180));
    g.drawRect(x, 0, width, height, 1);
  }

  void paintIcon(Graphics& g, int width, int height)
    {g.setColour(Colours::black); g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);}

  void paintIconTextAndProgression(Graphics& g, int width, int height)
  {
    paintIcon(g, width, height);
    width -= labelX;
    int textWidth = width;

    if (hasProgression && width > minWidthToDisplayProgression)
    {
      textWidth -= progressionColumnWidth;
      paintProgression(g, labelX + textWidth, progressionColumnWidth, height);
    }
    
    g.setColour(Colours::black);
    g.setFont(12);
    
    String str = getUniqueName();
    StringArray lines;
    lines.addTokens(str, T("\n"), NULL);
    for (int i = 0; i < lines.size(); ++i)
      g.drawText(lines[i], labelX, 20 * i, textWidth, 20, Justification::centredLeft, true);
  }

  virtual void paintItem(Graphics& g, int width, int height)
  {
    if (isSelected())
      g.fillAll(Colours::darkgrey);
    --height; // 1 px margin
    if (width > minWidthToDisplayTimes)
    {
      int w = width - 2 * timeColumnWidth;
      paintIconTextAndProgression(g, w, height);
      g.setColour(Colours::grey);
      g.setFont(12);
      g.drawText(absoluteTime, w, 0, timeColumnWidth, height, Justification::centredRight, false);
      g.drawText(relativeTime, w + timeColumnWidth, 0, timeColumnWidth, height, Justification::centredRight, false);
    }
    else
      paintIconTextAndProgression(g, width, height);
  }

  void setProgression(double progression, double progressionTotal, const String& unit)
  {
    progressionString = String(progression);
    if (progressionTotal)
      progressionString += T(" / ") + String(progressionTotal);
    if (unit.isNotEmpty())
      progressionString += T(" ") + unit;
    this->progression = progressionTotal ? progression / progressionTotal : -progression;
    hasProgression = true;
  }

  static String formatTime(double timeInSeconds)
  {
    jassert(timeInSeconds >= 0.0);
    if (!timeInSeconds)
      return T("0 s");

    int numSeconds = (int)timeInSeconds;

    if (timeInSeconds < 1e-5)
      return String((int)(timeInSeconds / 1e-9)) + T(" nanos");
    if (timeInSeconds < 1e-2)
      return String((int)(timeInSeconds / 1e-6)) + T(" micros");
    if (timeInSeconds < 10)
      return (numSeconds ? String(numSeconds) + T(" s ") : String::empty) + String((int)(timeInSeconds * 1000) % 1000) + T(" ms");

    String res;
    if (numSeconds > 3600)
    {
      int numHours = numSeconds / 3600;
      if (numHours > 24)
      {
        int numDays = numHours / 24;
        res += numDays == 1 ? T("1 day") : String(numDays) + T(" days");
      }
      if (res.isNotEmpty())
        res += T(" ");
      res += String(numHours % 24) + T(" hours");
    }
    if (numSeconds >= 60)
    {
      if (res.isNotEmpty())
        res += T(" ");
      res += String((numSeconds / 60) % 60) + T(" min");
    }
    if (res.isNotEmpty())
      res += T(" ");
    res += String(numSeconds % 60) + T(" s");
    return res;
  }

  void setStartTime(double time)
  {
    creationTime = time;
    this->absoluteTime = formatTime(creationTime);
  }

  void setEndTime(double time)
    {this->relativeTime = formatTime(time - creationTime);}

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
      numWorkUnits(workUnits->getNumWorkUnits()), numWorkUnitsDone(0) {}

  virtual void paintItem(Graphics& g, int width, int height)
  {
    setProgression((double)numWorkUnitsDone, (double)numWorkUnits, T("Work Units"));
    ExecutionTraceTreeViewItem::paintItem(g, width, height);
  }

  void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
    {++numWorkUnitsDone;}

private:
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
  ExecutionTraceTreeView(ExecutionTracePtr trace) : trace(trace)
  {
    DelayToUserInterfaceExecutionCallback::target = createTreeBuilderCallback();
    DelayToUserInterfaceExecutionCallback::setStaticAllocationFlag();
    trace->getContext().appendCallback(refCountedPointerFromThis(this));

    initialTime = Time::getCurrentTime().toMilliseconds() / 1000.0;
    setRootItem(new NodeExecutionTraceTreeViewItem(ObjectPtr(), String::empty));
    setRootItemVisible(false);
    setColour(backgroundColourId, Colours::white);
  }

  virtual ~ExecutionTraceTreeView()
  {
    trace->getContext().removeCallback(refCountedPointerFromThis(this));
    deleteRootItem();
  }

  NodeExecutionTraceTreeViewItem* getItemFromStack(const ExecutionStackPtr& stack) const
  {
    NodeExecutionTraceTreeViewItem* item = (NodeExecutionTraceTreeViewItem* )getRootItem();
    size_t n = stack->getDepth();
    for (size_t i = 0; i < n; ++i)
    {
      const WorkUnitPtr& workUnit = stack->getWorkUnit(i);
      bool ok = false;
      for (int i = 0; i < item->getNumSubItems(); ++i)
      {
        NodeExecutionTraceTreeViewItem* subItem = dynamic_cast<NodeExecutionTraceTreeViewItem* >(item->getSubItem(i));
        if (subItem && subItem->getWorkUnit() == workUnit)
        {
          item = subItem;
          ok = true;
          break;
        }
      }
    }
    return item;
  }

  double getInitialTime() const
    {return initialTime;}

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionTracePtr trace;
  double initialTime;

  ExecutionCallbackPtr createTreeBuilderCallback();
};

class ExecutionTraceTreeViewBuilderCallback : public ExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilderCallback(ExecutionTraceTreeView* tree)
    : tree(tree) {}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    Time creationTime = notification->getConstructionTime();
    currentNotificationTime = creationTime.toMilliseconds() / 1000.0 - tree->getInitialTime();
    ExecutionCallback::notificationCallback(notification);
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    ProgressExecutionTraceTreeViewItem* item = getOrCreateProgressTreeViewItem();
    jassert(item);
    item->setEndTime(currentNotificationTime);
    item->setUniqueName(currentStatus);
    item->setProgression(progression, progressionTotal, progressionUnit);
    item->treeHasChanged();
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
  {
    ExecutionTraceTreeViewItem* item;
    if (workUnit.dynamicCast<CompositeWorkUnit>())
      item = new CompositeWorkUnitExecutionTraceTreeViewItem(workUnit.dynamicCast<CompositeWorkUnit>());
    else
      item = new WorkUnitExecutionTraceTreeViewItem(workUnit);
    addItemAndPushPosition(stack, item);
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {
    TreeViewItem* treeItem = popPositionIntoTree(!result);
    CompositeWorkUnitExecutionTraceTreeViewItem* parentTreeItem = dynamic_cast<CompositeWorkUnitExecutionTraceTreeViewItem* >(treeItem->getParentItem());
    if (parentTreeItem)
    {
      parentTreeItem->postExecutionCallback(stack, workUnit, result);
      parentTreeItem->setEndTime(currentNotificationTime);
    }
  }
  
  virtual void informationCallback(const String& where, const String& what)
    {jassert(what.trim().isNotEmpty()); addItem(new InformationExecutionTraceTreeViewItem(what, where));}

  virtual void warningCallback(const String& where, const String& what)
    {addItem(new WarningExecutionTraceTreeViewItem(what, where));}

  virtual void errorCallback(const String& where, const String& what)
    {addItem(new ErrorExecutionTraceTreeViewItem(what, where));}

  virtual void statusCallback(const String& status)
    {currentStatus = status;}

protected:
  ExecutionTraceTreeView* tree;
  String currentStatus;
  double currentNotificationTime;

  void addItem(ExecutionTraceTreeViewItem* newItem)
    {addItem(getCurrentPositionInTree(), newItem);}

  void addItem(TreeViewItem* parentItem, ExecutionTraceTreeViewItem* newItem)
  {
    newItem->setStartTime(currentNotificationTime);
    parentItem->addSubItem(newItem);
    tree->scrollToKeepItemVisible(newItem);
  }

  ProgressExecutionTraceTreeViewItem* getOrCreateProgressTreeViewItem()
  {
    TreeViewItem* parent = getCurrentPositionInTree();
    jassert(parent);
    if (!parent->getNumSubItems())
      return NULL;
    ProgressExecutionTraceTreeViewItem* res = dynamic_cast<ProgressExecutionTraceTreeViewItem* >(parent->getSubItem(parent->getNumSubItems() - 1));
    if (!res)
      addItem(res = new ProgressExecutionTraceTreeViewItem());
    return res;
  }

protected:
  /*
  ** Positions Into Tree
  */
  std::vector<ExecutionTraceTreeViewItem* > positions;

  void addItemAndPushPosition(const ExecutionStackPtr& stack, ExecutionTraceTreeViewItem* item)
  {
    ExecutionTraceTreeViewItem* parentItem = tree->getItemFromStack(stack);
    jassert(parentItem);
    addItem(parentItem, item);
    pushPositionIntoTree(item);
  }

  void pushPositionIntoTree(ExecutionTraceTreeViewItem* item)
    {positions.push_back(item);}

  ExecutionTraceTreeViewItem* popPositionIntoTree(bool setErrorFlag = false)
  { 
    jassert(positions.size());
    ExecutionTraceTreeViewItem* res = positions.back();
    res->setEndTime(currentNotificationTime);
    if (setErrorFlag)
      res->setIcon(T("Error-32.png"));
    positions.pop_back();
    res->treeHasChanged();
    return res;
  }

  ExecutionTraceTreeViewItem* getCurrentPositionInTree() const
    {return positions.size() ? positions.back() : (ExecutionTraceTreeViewItem* )tree->getRootItem();}
};

class DispatchByThreadExecutionCallback : public CompositeExecutionCallback // to have default implementations that redirect to notificationCallback
{
public:
  virtual ExecutionCallbackPtr createCallbackForThread(Thread::ThreadID threadId) = 0;

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    Thread::ThreadID threadId = notification->getSourceThreadId();
    CallbackByThreadMap::const_iterator it = callbackByThread.find(threadId);
    ExecutionCallbackPtr threadCallback;
    if (it == callbackByThread.end())
      threadCallback = callbackByThread[threadId] = createCallbackForThread(threadId);
    else
      threadCallback = it->second;
    threadCallback->notificationCallback(notification);
  }

protected:
  typedef std::map<Thread::ThreadID, ExecutionCallbackPtr> CallbackByThreadMap;
  CallbackByThreadMap callbackByThread;
};

class ExecutionTraceTreeViewBuilderExecutionCallback : public DispatchByThreadExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilderExecutionCallback(ExecutionTraceTreeView* tree)
    : tree(tree) {}

  virtual ExecutionCallbackPtr createCallbackForThread(Thread::ThreadID threadId)
    {return new ExecutionTraceTreeViewBuilderCallback(tree);}

protected:
  ExecutionTraceTreeView* tree;
};

ExecutionCallbackPtr ExecutionTraceTreeView::createTreeBuilderCallback()
  {return new ExecutionTraceTreeViewBuilderExecutionCallback(this);}

juce::Component* ExecutionTrace::createComponent() const
  {return new ExecutionTraceTreeView(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
