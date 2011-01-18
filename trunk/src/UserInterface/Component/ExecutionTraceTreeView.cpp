/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.cpp     | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ExecutionTraceTreeView.h"
#include <lbcpp/Execution/ExecutionStack.h>
using namespace lbcpp;
using juce::Graphics;
using juce::Colour;

/*
** ExecutionTraceTreeView
*/
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace) : trace(trace), isSelectionUpToDate(false), isTreeUpToDate(true)
{
  DelayToUserInterfaceExecutionCallback::target = createTreeBuilderCallback();
  DelayToUserInterfaceExecutionCallback::setStaticAllocationFlag();
  ExecutionContextPtr context = trace->getContextPointer();
  if (context)
    context->appendCallback(refCountedPointerFromThis(this));

  initialTime = Time::getCurrentTime().toMilliseconds() / 1000.0;
  setRootItem(new ExecutionTraceTreeViewNode(this, trace->getRootNode()));
  getRootItem()->setOpen(true);
  setRootItemVisible(false);
  setColour(backgroundColourId, Colours::white);
  setMultiSelectEnabled(true);
}

ExecutionTraceTreeView::~ExecutionTraceTreeView()
{
  ExecutionContextPtr context = trace->getContextPointer();
  if (context)
    context->removeCallback(refCountedPointerFromThis(this));
  deleteRootItem();
}

ExecutionTraceTreeViewNode* ExecutionTraceTreeView::getItemFromStack(const ExecutionStackPtr& stack) const
{
  ExecutionTraceTreeViewNode* item = (ExecutionTraceTreeViewNode* )getRootItem();
  size_t n = stack->getDepth();
  for (size_t i = 0; i < n; ++i)
  {
    const WorkUnitPtr& workUnit = stack->getWorkUnit(i);
    bool ok = false;
    for (int i = 0; i < item->getNumSubItems(); ++i)
    {
      ExecutionTraceTreeViewNode* subItem = dynamic_cast<ExecutionTraceTreeViewNode* >(item->getSubItem(i));
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
        ExecutionTraceNodePtr trace = item->getTrace().dynamicCast<ExecutionTraceNode>();
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

  virtual void progressCallback(const ProgressionStatePtr& progression)
  {
    ExecutionTraceTreeViewNode* item = dynamic_cast<ExecutionTraceTreeViewNode* >(getCurrentPositionInTree());
    if (item)
    {
      ExecutionTraceNodePtr trace = item->getTrace();
      jassert(trace);
      trace->setEndTime(currentNotificationTime);
      trace->setProgression(progression);
      tree->invalidateTree();
    }
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {
    ExecutionTraceNodePtr trace(new ExecutionTraceNode(description, workUnit, currentNotificationTime));

    ExecutionTraceTreeViewItem* parentItem = tree->getItemFromStack(stack);
    jassert(parentItem);

    ExecutionTraceTreeViewNode* item = new ExecutionTraceTreeViewNode(tree, trace);
    addItem(parentItem, item);
    pushPositionIntoTree(item);
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, bool result)
    {popPositionFromTree(!result);}
  
  virtual void informationCallback(const String& where, const String& what)
    {addItem(new MessageExecutionTraceItem(currentNotificationTime, informationMessageType, what, where));}

  virtual void warningCallback(const String& where, const String& what)
    {addItem(new MessageExecutionTraceItem(currentNotificationTime, warningMessageType, what, where));}

  virtual void errorCallback(const String& where, const String& what)
    {addItem(new MessageExecutionTraceItem(currentNotificationTime, errorMessageType, what, where));}

  virtual void resultCallback(const String& name, const Variable& value)
  {
    ExecutionTraceNodePtr trace = getCurrentPositionInTree()->getTrace().dynamicCast<ExecutionTraceNode>();
    jassert(trace);
    trace->setResult(name, value);
  }

protected:
  ExecutionTraceTreeView* tree;
  double currentNotificationTime;


protected:
  /*
  ** Positions Into Tree
  */
  std::vector<ExecutionTraceTreeViewNode* > positions;

  void addItem(ExecutionTraceItemPtr trace)
    {addItem(getCurrentPositionInTree(), new ExecutionTraceTreeViewItem(tree, trace));}

  void addItem(TreeViewItem* parentItem, ExecutionTraceTreeViewItem* newItem)
  {
    parentItem->addSubItem(newItem);
    if (tree->getViewport()->getViewPositionY() + tree->getViewport()->getViewHeight() >= tree->getViewport()->getViewedComponent()->getHeight())
      tree->scrollToKeepItemVisible(newItem);
  }

  void pushPositionIntoTree(ExecutionTraceTreeViewNode* item)
    {positions.push_back(item);}

  ExecutionTraceTreeViewItem* popPositionFromTree(bool setErrorFlag = false)
  { 
    jassert(positions.size());
    ExecutionTraceTreeViewNode* res = positions.back();
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

class ExecutionTraceTreeViewBuilderExecutionCallback : public DispatchByThreadExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilderExecutionCallback(ExecutionTraceTreeView* tree)
    : tree(tree) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId)
    {return new ExecutionTraceTreeViewBuilderCallback(tree);}

protected:
  ExecutionTraceTreeView* tree;
};

ExecutionCallbackPtr ExecutionTraceTreeView::createTreeBuilderCallback()
  {return new ExecutionTraceTreeViewBuilderExecutionCallback(this);}

juce::Component* ExecutionTrace::createComponent() const
  {return new ExecutionTraceTreeView(refCountedPointerFromThis(this));}
