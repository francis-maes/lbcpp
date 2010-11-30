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
    if (numSeconds > 60)
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

class WorkUnitExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  WorkUnitExecutionTraceTreeViewItem(const WorkUnitPtr& workUnit)
    : ExecutionTraceTreeViewItem(workUnit->getName(), T("WorkUnit-32.png"), true)
  {
    setOpen(true);
  }
};

class WorkUnitVectorExecutionTraceTreeViewItem : public ExecutionTraceTreeViewItem
{
public:
  WorkUnitVectorExecutionTraceTreeViewItem(const WorkUnitVectorPtr& workUnits)
    : ExecutionTraceTreeViewItem(workUnits->getName(), T("WorkUnit-32.png"), true), numWorkUnits(workUnits->getNumWorkUnits()), numWorkUnitsDone(0)
    {setOpen(true);}

  virtual void paintItem(Graphics& g, int width, int height)
  {
    setProgression((double)numWorkUnitsDone, (double)numWorkUnits, T("Work Units"));
    ExecutionTraceTreeViewItem::paintItem(g, width, height);
  }

  void postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
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

class ExecutionTraceTreeViewBuilder : public ExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilder(TreeView* tree = NULL)
    : tree(tree), root(NULL), lastCreatedItem(NULL)
  {
    initialTime = Time::getCurrentTime().toMilliseconds() / 1000.0;
  }

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    currentNotificationConstructionTime = notification->getConstructionTime();
    currentNotificationSourceThreadId = notification->getSourceThreadId();
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

  virtual void preExecutionCallback(const WorkUnitPtr& workUnit)
  {
    ExecutionTraceTreeViewItem* node = new WorkUnitExecutionTraceTreeViewItem(workUnit);
    addItem(node);
    stack.push_back(std::make_pair(node, currentNotificationConstructionTime.toMilliseconds() / 1000.0));
  }

  virtual void postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
  {
    jassert(stack.size());
    WorkUnitExecutionTraceTreeViewItem* treeItem = dynamic_cast<WorkUnitExecutionTraceTreeViewItem* >(stack.back().first);
    jassert(treeItem);
    if (!result)
      treeItem->setIcon(T("Error-32.png"));
    stack.pop_back();

    WorkUnitVectorExecutionTraceTreeViewItem* parentTreeItem = dynamic_cast<WorkUnitVectorExecutionTraceTreeViewItem* >(getCurrentParent());
    if (parentTreeItem)
      parentTreeItem->postExecutionCallback(workUnit, result);

    lastCreatedItem = NULL;
  }

  virtual void preExecutionCallback(const WorkUnitVectorPtr& workUnits)
  {
    ExecutionTraceTreeViewItem* node = new WorkUnitVectorExecutionTraceTreeViewItem(workUnits);
    addItem(node);
    stack.push_back(std::make_pair(node, currentNotificationConstructionTime.toMilliseconds() / 1000.0));
  }

  virtual void postExecutionCallback(const WorkUnitVectorPtr& workUnits, bool result)
  {
    jassert(stack.size());
    WorkUnitVectorExecutionTraceTreeViewItem* treeItem = dynamic_cast<WorkUnitVectorExecutionTraceTreeViewItem* >(stack.back().first);
    jassert(treeItem);
    if (!result)
      treeItem->setIcon(T("Error-32.png"));
    stack.pop_back();
    lastCreatedItem = NULL;
  }
  
  virtual void informationCallback(const String& where, const String& what)
    {addItem(new InformationExecutionTraceTreeViewItem(what, where));}

  virtual void warningCallback(const String& where, const String& what)
    {addItem(new WarningExecutionTraceTreeViewItem(what, where));}

  virtual void errorCallback(const String& where, const String& what)
    {addItem(new ErrorExecutionTraceTreeViewItem(what, where));}

  virtual void statusCallback(const String& status)
    {currentStatus = status;}

protected:
  TreeView* tree;
  TreeViewItem* root;
  double initialTime;
  std::vector< std::pair<TreeViewItem* , double> > stack;
  TreeViewItem* lastCreatedItem;
  String currentStatus;

  Time currentNotificationConstructionTime;
  Thread::ThreadID currentNotificationSourceThreadId;

  void setTimes(ExecutionTraceTreeViewItem* item)
  {
    double time = currentNotificationConstructionTime.toMilliseconds() / 1000.0;
    item->setTimes(time - initialTime, time - (stack.empty() ? initialTime : stack.back().second));
  }

  TreeViewItem* getCurrentParent() const
    {return stack.empty() ? root : stack.back().first;}

  void addItem(ExecutionTraceTreeViewItem* newItem)
  {
    setTimes(newItem);
    lastCreatedItem = newItem;
    getCurrentParent()->addSubItem(newItem);
    tree->scrollToKeepItemVisible(newItem);
  }

  ProgressExecutionTraceTreeViewItem* getOrCreateProgressTreeViewItem()
  {
    ProgressExecutionTraceTreeViewItem* res = dynamic_cast<ProgressExecutionTraceTreeViewItem* >(lastCreatedItem);
    if (!res)
      addItem(res = new ProgressExecutionTraceTreeViewItem());
    return res;
  }
};

class ExecutionTraceTreeView : public TreeView, public ExecutionTraceTreeViewBuilder
{
public:
  ExecutionTraceTreeView()
  {
    ExecutionTraceTreeViewBuilder::tree = this;
    setRootItem(root = new SimpleTreeViewItem(T("root"), 0, true));
    root->setOpen(true);
    setRootItemVisible(false);
    setColour(backgroundColourId, Colours::white);
  }

  virtual ~ExecutionTraceTreeView()
    {deleteRootItem();}

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
