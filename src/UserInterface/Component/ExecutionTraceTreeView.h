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
    : SimpleTreeViewItem(name, iconToUse, mightContainSubItems)
  {
  }

  enum
  {
    minWidthToDisplayTimes = 300,
    timeColumnWidth = 100,
  };
 
  void paintIconAndText(Graphics& g, int width, int height)
  {
    g.setColour(Colours::black);
    int x1 = 0;
    if (iconToUse)
    {
      g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);
      x1 += iconToUse->getWidth() + 5;
    }
    g.drawText(getUniqueName(), x1, 0, width - x1, height, Justification::centredLeft, true);
  }

  virtual void paintItem(Graphics& g, int width, int height)
  {
    if (isSelected())
      g.fillAll(Colours::darkgrey);
    if (width > minWidthToDisplayTimes)
    {
      int w = width - 2 * timeColumnWidth;
      paintIconAndText(g, w, height);
      g.setColour(Colours::lightgrey);
      g.setFont(12);
      g.drawText(absoluteTime, w, 0, timeColumnWidth, height, Justification::centredRight, false);
      g.drawText(relativeTime, w + timeColumnWidth, 0, timeColumnWidth, height, Justification::centredRight, false);
    }
    else
      paintIconAndText(g, width, height);
  }

  void setTimes(double absoluteTime, double relativeTime)
  {
    this->absoluteTime = formatTime(absoluteTime);
    this->relativeTime = formatTime(relativeTime);
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

  void setFiniteProgression(double normalizedProgression)
  {
  }

  void setInfiniteProgression(double progression)
  {

  }
};

class ExecutionTraceTreeView : public TreeView
{
public:
  ExecutionTraceTreeView(CompositeExecutionCallback* parentCallback) : root(NULL), lastCreatedItem(NULL)
  {
    parentCallback->appendCallback(notifierExecutionCallback(parentCallback->getContext(),
        userInterfaceManager().getNotificationQueue(), callback = new Callback(this)));
    setRootItem(root = new SimpleTreeViewItem(T("root"), 0, true));
    root->setOpen(true);
    setRootItemVisible(false);
    initialTime = Time::getMillisecondCounterHiRes() / 1000.0;

    setColour(backgroundColourId, Colours::white);
  }

  virtual ~ExecutionTraceTreeView()
  {
    callback->owner = NULL;
    deleteRootItem();
  }

protected:
  struct Callback : public ExecutionCallback
  {
    Callback(ExecutionTraceTreeView* owner)
      : owner(owner) {}

    virtual void informationCallback(const String& where, const String& what)
      {if (owner) owner->addItem(new InformationExecutionTraceTreeViewItem(what, where));}

    virtual void warningCallback(const String& where, const String& what)
      {if (owner) owner->addItem(new WarningExecutionTraceTreeViewItem(what, where));}

    virtual void errorCallback(const String& where, const String& what)
      {if (owner) owner->addItem(new ErrorExecutionTraceTreeViewItem(what, where));}

    virtual void statusCallback(const String& status)
      {if (owner) owner->setStatus(status);}

    virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
      {if (owner) owner->setProgression(progression, progressionTotal, progressionUnit);}

    virtual void preExecutionCallback(const WorkUnitPtr& workUnit)
      {if (owner) owner->preExecutionCallback(workUnit);}

    virtual void postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
      {if (owner) owner->postExecutionCallback(workUnit, result);}

    ExecutionTraceTreeView* owner;
  };
  typedef ReferenceCountedObjectPtr<Callback> CallbackPtr;

  CallbackPtr callback;
  TreeViewItem* root;
  double initialTime;
  std::vector< std::pair<TreeViewItem* , double> > stack;
  TreeViewItem* lastCreatedItem;
  String currentStatus;

  void setStatus(const String& status)
  {
    currentStatus = status;
  }

  void setTimes(ExecutionTraceTreeViewItem* item)
  {
    double time = Time::getMillisecondCounterHiRes() / 1000.0;
    item->setTimes(time - initialTime, time - (stack.empty() ? initialTime : stack.back().second));
  }

  void addItem(ExecutionTraceTreeViewItem* newItem)
  {
    setTimes(newItem);
    lastCreatedItem = newItem;
    TreeViewItem* parent = stack.empty() ? root : stack.back().first;
    parent->addSubItem(newItem);
    scrollToKeepItemVisible(newItem);
  }

  ProgressExecutionTraceTreeViewItem* getOrCreateProgressTreeViewItem()
  {
    ProgressExecutionTraceTreeViewItem* res = dynamic_cast<ProgressExecutionTraceTreeViewItem* >(lastCreatedItem);
    if (!res)
      addItem(res = new ProgressExecutionTraceTreeViewItem());
    return res;
  }

  void setProgression(double progression, double progressionTotal, const String& progressionUnit)
  {
    String progressionString = currentStatus;
    if (progressionString.isNotEmpty())
      progressionString += ' ';
    progressionString += String(progression);
    if (progressionTotal)
      progressionString += T(" / ") + String(progressionTotal);
    if (progressionUnit.isNotEmpty())
      progressionString += T(" ") + progressionUnit;
    ProgressExecutionTraceTreeViewItem* item = getOrCreateProgressTreeViewItem();
    jassert(item);
    setTimes(item);
    item->setUniqueName(progressionString);
    if (progressionTotal)
      item->setFiniteProgression(progression / progressionTotal);
    else
      item->setInfiniteProgression(progression);
    item->treeHasChanged();
  }

  void preExecutionCallback(const WorkUnitPtr& workUnit)
  {
    ExecutionTraceTreeViewItem* node = new WorkUnitExecutionTraceTreeViewItem(workUnit);
    addItem(node);
    stack.push_back(std::make_pair(node, Time::getMillisecondCounterHiRes() / 1000.0));
  }

  void postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
  {
    jassert(stack.size());
    WorkUnitExecutionTraceTreeViewItem* treeItem = dynamic_cast<WorkUnitExecutionTraceTreeViewItem* >(stack.back().first);
    jassert(treeItem);
    if (!result)
      treeItem->setIcon(T("Error-32.png"));
    stack.pop_back();
    lastCreatedItem = NULL;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_H_
