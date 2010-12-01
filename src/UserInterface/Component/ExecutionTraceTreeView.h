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
    g.drawText(getUniqueName(), labelX, 0, textWidth, height, Justification::centredLeft, true);
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

  void setTimes(double absoluteTime, double relativeTime)
  {
    this->absoluteTime = formatTime(absoluteTime);
    this->relativeTime = formatTime(relativeTime);
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

protected:
  String absoluteTime;
  String relativeTime;

  bool hasProgression;
  double progression;
  String progressionString;
};

class NodeExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  NodeExecutionTraceTreeViewItem(const ObjectPtr& object, const String& iconToUse)
    : ExecutionTraceTreeViewItem(object ? object->getName() : String::empty, iconToUse, true), object(object)
    {setOpen(true);}

  const ObjectPtr& getObject() const
    {return object;}

protected:
  ObjectPtr object; // WorkUnitVector, WorkUnit, Inference, ...
};

class WorkUnitExecutionTraceTreeViewItem : public NodeExecutionTraceTreeViewItem
{
public:
  WorkUnitExecutionTraceTreeViewItem(const WorkUnitPtr& workUnit)
    : NodeExecutionTraceTreeViewItem(workUnit, T("WorkUnit-32.png")) {}
};

class WorkUnitVectorExecutionTraceTreeViewItem : public NodeExecutionTraceTreeViewItem
{
public:
  WorkUnitVectorExecutionTraceTreeViewItem(const WorkUnitVectorPtr& workUnits)
    : NodeExecutionTraceTreeViewItem(workUnits, T("WorkUnit-32.png")),
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
    : ExecutionTraceTreeViewItem(what + (where.isNotEmpty() ? T(" (") + where + T(")") : String::empty), iconToUse, false) {}
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

class ExecutionTraceTreeView : public TreeView, public ExecutionCallback
{
public:
  ExecutionTraceTreeView()
  {
    setRootItem(new NodeExecutionTraceTreeViewItem(ObjectPtr(), String::empty));
    setRootItemVisible(false);
    setColour(backgroundColourId, Colours::white);
  }

  virtual ~ExecutionTraceTreeView()
    {deleteRootItem();}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    Thread::ThreadID threadId = notification->getSourceThreadId();
    CallbackByThreadMap::const_iterator it = callbackByThread.find(threadId);
    ExecutionCallbackPtr threadCallback;
    if (it == callbackByThread.end())
      threadCallback = callbackByThread[threadId] = createBuilderCallback();
    else
      threadCallback = it->second;
    threadCallback->notificationCallback(notification);
  }

  NodeExecutionTraceTreeViewItem* getItemFromStack(const ExecutionStackPtr& stack) const
  {
    NodeExecutionTraceTreeViewItem* item = (NodeExecutionTraceTreeViewItem* )getRootItem();
    size_t n = stack->getDepth();
    for (size_t i = 0; i < n; ++i)
    {
      const ObjectPtr& element = stack->getElement(i);
      if (element.dynamicCast<WorkUnit>() || element.dynamicCast<WorkUnitVector>())
      {
        bool ok = false;
        for (int i = 0; i < item->getNumSubItems(); ++i)
        {
          NodeExecutionTraceTreeViewItem* subItem = dynamic_cast<NodeExecutionTraceTreeViewItem* >(item->getSubItem(i));
          if (subItem && subItem->getObject() == element)
          {
            item = subItem;
            ok = true;
            break;
          }
        }
        jassert(ok);
      }
    }
    return item;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  typedef std::map<Thread::ThreadID, ExecutionCallbackPtr> CallbackByThreadMap;
  CallbackByThreadMap callbackByThread;

  ExecutionCallbackPtr createBuilderCallback();
};

class ExecutionTraceTreeViewBuilderCallback : public ExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilderCallback(ExecutionTraceTreeView* tree)
    : tree(tree)
    {initialTime = Time::getCurrentTime().toMilliseconds() / 1000.0;}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    currentNotificationConstructionTime = notification->getConstructionTime();
    ExecutionCallback::notificationCallback(notification);
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    ProgressExecutionTraceTreeViewItem* item = getOrCreateProgressTreeViewItem();
    jassert(item);
    setTimes(item);
    item->setUniqueName(currentStatus);
    item->setProgression(progression, progressionTotal, progressionUnit);
    item->treeHasChanged();
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
    {addItemAndPushPosition(stack, new WorkUnitExecutionTraceTreeViewItem(workUnit));}

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {
    TreeViewItem* treeItem = popPositionIntoTree(!result);
    WorkUnitVectorExecutionTraceTreeViewItem* parentTreeItem = dynamic_cast<WorkUnitVectorExecutionTraceTreeViewItem* >(treeItem->getParentItem());
    if (parentTreeItem)
      parentTreeItem->postExecutionCallback(stack, workUnit, result);
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitVectorPtr& workUnits)
    {addItemAndPushPosition(stack, new WorkUnitVectorExecutionTraceTreeViewItem(workUnits));}

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitVectorPtr& workUnits, bool result)
    {popPositionIntoTree(!result);}
  
  virtual void informationCallback(const String& where, const String& what)
    {addItem(new InformationExecutionTraceTreeViewItem(what, where));}

  virtual void warningCallback(const String& where, const String& what)
    {addItem(new WarningExecutionTraceTreeViewItem(what, where));}

  virtual void errorCallback(const String& where, const String& what)
    {addItem(new ErrorExecutionTraceTreeViewItem(what, where));}

  virtual void statusCallback(const String& status)
    {currentStatus = status;}

protected:
  ExecutionTraceTreeView* tree;
  double initialTime;
  String currentStatus;

  Time currentNotificationConstructionTime;

  void setTimes(ExecutionTraceTreeViewItem* item)
  {
    //double time = currentNotificationConstructionTime.toMilliseconds() / 1000.0;
    item->setTimes(0.0, 0.0); // time - initialTime, time - (stack.empty() ? initialTime : stack.back().second));
  }

  void addItem(ExecutionTraceTreeViewItem* newItem)
    {addItem(getCurrentPositionInTree(), newItem);}

  void addItem(TreeViewItem* parentItem, ExecutionTraceTreeViewItem* newItem)
  {
    setTimes(newItem);
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
    if (setErrorFlag)
      res->setIcon(T("Error-32.png"));
    positions.pop_back();
    return res;
  }

  ExecutionTraceTreeViewItem* getCurrentPositionInTree() const
    {return positions.size() ? positions.back() : (ExecutionTraceTreeViewItem* )tree->getRootItem();}
};

ExecutionCallbackPtr ExecutionTraceTreeView::createBuilderCallback()
  {return new ExecutionTraceTreeViewBuilderCallback(this);}

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
