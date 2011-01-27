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
  DelayToUserInterfaceExecutionCallback::setStaticAllocationFlag();
  DelayToUserInterfaceExecutionCallback::target = createTreeBuilderCallback();
  ExecutionCallbackPtr pthis((DelayToUserInterfaceExecutionCallback* )this);

  ExecutionContextPtr context = trace->getContextPointer();
  if (context)
    context->appendCallback(pthis);

  setRootItem(new ExecutionTraceTreeViewNode(this, trace->getRootNode(), 0));
  getRootItem()->setOpen(true);
  setRootItemVisible(false);
  setColour(backgroundColourId, Colours::white);
  setMultiSelectEnabled(true);
}

ExecutionTraceTreeView::~ExecutionTraceTreeView()
{
  ExecutionContextPtr context = trace->getContextPointer();
  if (context)
  {
    ExecutionCallbackPtr pthis((DelayToUserInterfaceExecutionCallback* )this);
    context->removeCallback(pthis);
  }
  deleteRootItem();
}

ExecutionTraceTreeViewNode* ExecutionTraceTreeView::getNodeFromStack(const ExecutionStackPtr& stack) const
{
  ExecutionTraceTreeViewNode* item = (ExecutionTraceTreeViewNode* )getRootItem();
  size_t n = stack->getDepth();
  for (size_t i = 0; i < n; ++i)
  {
    const std::pair<String, WorkUnitPtr>& entry = stack->getEntry(i);
    bool ok = false;
    for (int i = 0; i < item->getNumSubItems(); ++i)
    {
      ExecutionTraceTreeViewNode* subItem = dynamic_cast<ExecutionTraceTreeViewNode* >(item->getSubItem(i));
      if (subItem)
      {
        const ExecutionTraceNodePtr& traceNode = subItem->getTraceNode();
        if (entry.second)
        {
          if (traceNode->getWorkUnit() == entry.second)
          {
            item = subItem;
            ok = true;
            break;
          }
        }
        else
        {
          if (traceNode->toString() == entry.first)
          {
            item = subItem;
            ok = true;
            break;
          }
        }
      }
    }
    if (!ok)
      return NULL;
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
    String selectionName;
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
          {
            selectedVariables.push_back(results);
            if (!selectionName.isEmpty())
              selectionName += T(", ");
            selectionName += trace->toString() + T(" results");
          }
        }
      }
    }
    sendSelectionChanged(selectedVariables, selectionName);
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

#include "../../Execution/Callback/MakeTraceExecutionCallback.h"

class MakeTraceAndFillTreeThreadExecutionCallback : public MakeTraceThreadExecutionCallback
{
public:
  MakeTraceAndFillTreeThreadExecutionCallback(ExecutionTraceTreeView* tree, ExecutionTracePtr trace, ExecutionTraceNodePtr traceNode, ExecutionTraceTreeViewNode* node)
    : MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime()), tree(tree), stack(1, node) {}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {
    MakeTraceThreadExecutionCallback::preExecutionCallback(stack, description, workUnit);
    ExecutionTraceTreeViewNode* node = this->stack.back();
    ExecutionTraceTreeViewNode* newNode = NULL;
    if (node && node->hasBeenOpenedOnce())
      newNode = dynamic_cast<ExecutionTraceTreeViewNode* >(node->getSubItem(node->getNumSubItems() - 1));
    this->stack.push_back(newNode);
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const Variable& result)
  {
    this->stack.pop_back();
    MakeTraceThreadExecutionCallback::postExecutionCallback(stack, description, workUnit, result);
    tree->invalidateTree();
  }

  virtual void progressCallback(const ProgressionStatePtr& progression)
  {
    MakeTraceThreadExecutionCallback::progressCallback(progression);
    tree->invalidateTree();
  }

protected:
  ExecutionTraceTreeView* tree;
  std::vector<ExecutionTraceTreeViewNode* > stack;

  virtual void appendTraceItem(ExecutionTraceItemPtr item)
  {
    MakeTraceThreadExecutionCallback::appendTraceItem(item);
    ExecutionTraceTreeViewNode* parent = stack.back();
    if (parent && parent->hasBeenOpenedOnce())
    {
      ExecutionTraceTreeViewItem* newItem = ExecutionTraceTreeViewItem::create(tree, item, parent->getDepth() + 1);
      stack.back()->addSubItem(newItem);
      if (tree->getViewport()->getViewPositionY() + tree->getViewport()->getViewHeight() >= tree->getViewport()->getViewedComponent()->getHeight())
        tree->scrollToKeepItemVisible(newItem);
    }
  }
};

class ExecutionTraceTreeViewBuilderExecutionCallback : public DispatchByThreadExecutionCallback
{
public:
  ExecutionTraceTreeViewBuilderExecutionCallback(ExecutionTraceTreeView* tree)
    : tree(tree) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId)
  {
    ExecutionTracePtr trace = tree->getTrace();
    ExecutionTraceNodePtr traceNode = trace->findNode(stack);
    jassert(traceNode);
    ExecutionTraceTreeViewNode* node = tree->getNodeFromStack(stack);
    return node ? ExecutionCallbackPtr(new MakeTraceAndFillTreeThreadExecutionCallback(tree, trace, traceNode, node)) : ExecutionCallbackPtr();
  }

protected:
  ExecutionTraceTreeView* tree;
};

ExecutionCallbackPtr ExecutionTraceTreeView::createTreeBuilderCallback()
  {return new ExecutionTraceTreeViewBuilderExecutionCallback(this);}

juce::Component* ExecutionTrace::createComponent() const
  {return new ExecutionTraceTreeView(refCountedPointerFromThis(this));}
