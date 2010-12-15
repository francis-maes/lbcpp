/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.cpp     | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ExecutionTraceTreeView.h"
using namespace lbcpp;

/*
** ExecutionTraceTreeViewItem
*/
ExecutionTraceTreeViewItem::ExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace)
  : SimpleTreeViewItem(trace->toString(), trace->getPreferedIcon(), false), owner(owner), trace(trace)
{
  String str = getUniqueName();
  numLines = 1;
  for (int i = 0; i < str.length() - 1; ++i)
    if (str[i] == '\n')
      ++numLines;
}

void ExecutionTraceTreeViewItem::itemSelectionChanged(bool isNowSelected)
  {owner->invalidateSelection();}

void ExecutionTraceTreeViewItem::paintProgression(Graphics& g, WorkUnitExecutionTraceItemPtr workUnitTrace, int x, int width, int height)
{
  juce::GradientBrush brush(Colour(200, 220, 240), (float)x, -(float)width / 3.f, Colours::white, (float)width, (float)width, true);

  double progression = workUnitTrace->getProgression();

  g.setBrush(&brush);
  if (progression > 0.0)
  {
    if (progression > 1.0)
      progression = 1.0;
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
  g.drawText(workUnitTrace->getProgressionString(), x, 0, width, height, Justification::centred, true);

  g.setColour(Colour(180, 180, 180));
  g.drawRect(x, 0, width, height, 1);
}

void ExecutionTraceTreeViewItem::paintIcon(Graphics& g, int width, int height)
  {g.setColour(Colours::black); g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);}

void ExecutionTraceTreeViewItem::paintIconTextAndProgression(Graphics& g, int width, int height)
{
  paintIcon(g, width, height);
  width -= labelX;
  int textWidth = width;

  WorkUnitExecutionTraceItemPtr workUnitTrace = getTrace().dynamicCast<WorkUnitExecutionTraceItem>();
  if (workUnitTrace && workUnitTrace->isProgressionAvailable() && width > minWidthToDisplayProgression)
  {
    textWidth -= progressionColumnWidth;
    paintProgression(g, workUnitTrace, labelX + textWidth, progressionColumnWidth, height);
  }
  
  g.setColour(Colours::black);
  g.setFont(12);
  
  String str = getUniqueName();
  StringArray lines;
  lines.addTokens(str, T("\n"), NULL);
  for (int i = 0; i < lines.size(); ++i)
    g.drawText(lines[i], labelX, 20 * i, textWidth, 20, Justification::centredLeft, true);
}

void ExecutionTraceTreeViewItem::paintItem(Graphics& g, int width, int height)
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

    
    g.drawText(formatTime(trace->getTime()), w, 0, timeColumnWidth, height, Justification::centredRight, false);
    WorkUnitExecutionTraceItemPtr workUnitTrace = trace.dynamicCast<WorkUnitExecutionTraceItem>();
    if (workUnitTrace)
      g.drawText(formatTime(workUnitTrace->getTimeLength()), w + timeColumnWidth, 0, timeColumnWidth, height, Justification::centredRight, false);
  }
  else
    paintIconTextAndProgression(g, width, height);
}

String ExecutionTraceTreeViewItem::formatTime(double timeInSeconds)
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

/*
** ExecutionTraceTreeView
*/
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace) : trace(trace), isSelectionUpToDate(false), isTreeUpToDate(true)
{
  DelayToUserInterfaceExecutionCallback::target = createTreeBuilderCallback();
  DelayToUserInterfaceExecutionCallback::setStaticAllocationFlag();
  trace->getContext().appendCallback(refCountedPointerFromThis(this));

  initialTime = Time::getCurrentTime().toMilliseconds() / 1000.0;
  setRootItem(new WorkUnitExecutionTraceTreeViewItem(this, new WorkUnitExecutionTraceItem(WorkUnitPtr(), initialTime)));
  setRootItemVisible(false);
  setColour(backgroundColourId, Colours::white);
  setMultiSelectEnabled(true);
}

ExecutionTraceTreeView::~ExecutionTraceTreeView()
{
  trace->getContext().removeCallback(refCountedPointerFromThis(this));
  deleteRootItem();
}

WorkUnitExecutionTraceTreeViewItem* ExecutionTraceTreeView::getItemFromStack(const ExecutionStackPtr& stack) const
{
  WorkUnitExecutionTraceTreeViewItem* item = (WorkUnitExecutionTraceTreeViewItem* )getRootItem();
  size_t n = stack->getDepth();
  for (size_t i = 0; i < n; ++i)
  {
    const WorkUnitPtr& workUnit = stack->getWorkUnit(i);
    bool ok = false;
    for (int i = 0; i < item->getNumSubItems(); ++i)
    {
      WorkUnitExecutionTraceTreeViewItem* subItem = dynamic_cast<WorkUnitExecutionTraceTreeViewItem* >(item->getSubItem(i));
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

void ExecutionTraceTreeView::timerCallback()
{
  DelayToUserInterfaceExecutionCallback::timerCallback();
  if (!isSelectionUpToDate)
  {
    std::vector<Variable> selectedVariables;
    selectedVariables.reserve(getNumSelectedItems());
    for (int i = 0; i < getNumSelectedItems(); ++i)
    {
      ExecutionTraceTreeViewItem* item = dynamic_cast<ExecutionTraceTreeViewItem* >(getSelectedItem(i));
      if (item && item != getRootItem())
      {
        WorkUnitExecutionTraceItemPtr trace = item->getTrace().dynamicCast<WorkUnitExecutionTraceItem>();
        if (trace)
        {
          ObjectPtr results = trace->getResultsObject(defaultExecutionContext());
          if (results)
            selectedVariables.push_back(results);
        }
      }
    }
    sendSelectionChanged(selectedVariables);
    isSelectionUpToDate = true;
  }
  if (!isTreeUpToDate)
  {
    repaint();
    isTreeUpToDate = true;
  }
}

void ExecutionTraceTreeView::invalidateSelection()
  {isSelectionUpToDate = false;}

void ExecutionTraceTreeView::invalidateTree()
  {isTreeUpToDate = false;}

int ExecutionTraceTreeView::getDefaultWidth() const
  {return 600;}


/*
** ExecutionTraceTreeViewBuilderCallback
*/
class ExecutionTraceTreeViewBuilderCallback : public ExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilderCallback(ExecutionTraceTreeView* tree)
    : tree(tree), currentNotificationTime(0) {}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    Time creationTime = notification->getConstructionTime();
    double time = creationTime.toMilliseconds() / 1000.0 - tree->getInitialTime();
    jassert(time >= currentNotificationTime);
    currentNotificationTime = time;
    ExecutionCallback::notificationCallback(notification);
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    WorkUnitExecutionTraceTreeViewItem* item = dynamic_cast<WorkUnitExecutionTraceTreeViewItem* >(getCurrentPositionInTree());
    if (item)
    {
      WorkUnitExecutionTraceItemPtr trace = item->getTrace();
      jassert(trace);
      trace->setEndTime(currentNotificationTime);
      trace->setProgression(progression, progressionTotal, progressionUnit);
      tree->invalidateTree();
    }
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
  {
    WorkUnitExecutionTraceItemPtr trace(new WorkUnitExecutionTraceItem(workUnit, currentNotificationTime));
    addItemAndPushPosition(stack, new WorkUnitExecutionTraceTreeViewItem(tree, trace));
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {
  //  TreeViewItem* treeItem = popPositionIntoTree(!result);
    currentStatus = String::empty;
  }
  
  virtual void informationCallback(const String& where, const String& what)
    {addItem(new MessageExecutionTraceItem(currentNotificationTime, informationMessageType, what, where));}

  virtual void warningCallback(const String& where, const String& what)
    {addItem(new MessageExecutionTraceItem(currentNotificationTime, warningMessageType, what, where));}

  virtual void errorCallback(const String& where, const String& what)
    {addItem(new MessageExecutionTraceItem(currentNotificationTime, errorMessageType, what, where));}

  virtual void statusCallback(const String& status)
    {currentStatus = status;}

  virtual void resultCallback(const String& name, const Variable& value)
  {
    WorkUnitExecutionTraceItemPtr trace = getCurrentPositionInTree()->getTrace().dynamicCast<WorkUnitExecutionTraceItem>();
    jassert(trace);
    trace->setResult(name, value);
  }

protected:
  ExecutionTraceTreeView* tree;
  String currentStatus;
  double currentNotificationTime;


protected:
  /*
  ** Positions Into Tree
  */
  std::vector<WorkUnitExecutionTraceTreeViewItem* > positions;

  void addItem(ExecutionTraceItemPtr trace)
    {addItem(getCurrentPositionInTree(), new ExecutionTraceTreeViewItem(tree, trace));}

  void addItem(TreeViewItem* parentItem, ExecutionTraceTreeViewItem* newItem)
  {
    parentItem->addSubItem(newItem);
    if (tree->getViewport()->getViewPositionY() + tree->getViewport()->getViewHeight() >= tree->getViewport()->getViewedComponent()->getHeight())
      tree->scrollToKeepItemVisible(newItem);
  }

  void addItemAndPushPosition(const ExecutionStackPtr& stack, WorkUnitExecutionTraceTreeViewItem* item)
  {
    ExecutionTraceTreeViewItem* parentItem = tree->getItemFromStack(stack);
    jassert(parentItem);
    addItem(parentItem, item);
    pushPositionIntoTree(item);
  }

  void pushPositionIntoTree(WorkUnitExecutionTraceTreeViewItem* item)
    {positions.push_back(item);}

  ExecutionTraceTreeViewItem* popPositionIntoTree(bool setErrorFlag = false)
  { 
    jassert(positions.size());
    WorkUnitExecutionTraceTreeViewItem* res = positions.back();
    res->getTrace()->setEndTime(currentNotificationTime);
    if (setErrorFlag)
      res->setIcon(T("Error-32.png"));
    positions.pop_back();
    tree->invalidateTree();
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
    //DBG(String((int)threadId) + T(" ") + notification->getClassName());
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
