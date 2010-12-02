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
ExecutionTraceTreeViewItem::ExecutionTraceTreeViewItem(const String& name, const String& iconToUse, bool mightContainSubItems)
  : SimpleTreeViewItem(name, iconToUse, mightContainSubItems), hasProgression(false), progression(0.0)
{
}

void ExecutionTraceTreeViewItem::paintProgression(Graphics& g, int x, int width, int height)
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

void ExecutionTraceTreeViewItem::paintIcon(Graphics& g, int width, int height)
  {g.setColour(Colours::black); g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);}

void ExecutionTraceTreeViewItem::paintIconTextAndProgression(Graphics& g, int width, int height)
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
    g.drawText(absoluteTime, w, 0, timeColumnWidth, height, Justification::centredRight, false);
    g.drawText(relativeTime, w + timeColumnWidth, 0, timeColumnWidth, height, Justification::centredRight, false);
  }
  else
    paintIconTextAndProgression(g, width, height);
}

void ExecutionTraceTreeViewItem::setProgression(double progression, double progressionTotal, const String& unit)
{
  progressionString = String(progression);
  if (progressionTotal)
    progressionString += T(" / ") + String(progressionTotal);
  if (unit.isNotEmpty())
    progressionString += T(" ") + unit;
  this->progression = progressionTotal ? progression / progressionTotal : -progression;
  hasProgression = true;
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

void ExecutionTraceTreeViewItem::setStartTime(double time)
{
  creationTime = time;
  this->absoluteTime = formatTime(creationTime);
}

void ExecutionTraceTreeViewItem::setEndTime(double time)
  {this->relativeTime = formatTime(time - creationTime);}

/*
** ExecutionTraceTreeView
*/
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace) : trace(trace)
{
  DelayToUserInterfaceExecutionCallback::target = createTreeBuilderCallback();
  DelayToUserInterfaceExecutionCallback::setStaticAllocationFlag();
  trace->getContext().appendCallback(refCountedPointerFromThis(this));

  initialTime = Time::getCurrentTime().toMilliseconds() / 1000.0;
  setRootItem(new NodeExecutionTraceTreeViewItem(ObjectPtr(), String::empty));
  setRootItemVisible(false);
  setColour(backgroundColourId, Colours::white);
}

ExecutionTraceTreeView::~ExecutionTraceTreeView()
{
  trace->getContext().removeCallback(refCountedPointerFromThis(this));
  deleteRootItem();
}

NodeExecutionTraceTreeViewItem* ExecutionTraceTreeView::getItemFromStack(const ExecutionStackPtr& stack) const
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
    DBG(String((int)threadId) + T(" ") + notification->getClassName());
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
