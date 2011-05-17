/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.cpp     | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ExecutionTraceTreeView.h"
#include "ContainerCurveEditor.h"
#include <lbcpp/Execution/ExecutionStack.h>
using namespace lbcpp;
using juce::Graphics;
using juce::Colour;

/*
** ExecutionTraceTreeView
*/
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace, ExecutionContextPtr context)
  : trace(trace), isSelectionUpToDate(false), isTreeUpToDate(true)
{
  DelayToUserInterfaceExecutionCallback::setStaticAllocationFlag();
  DelayToUserInterfaceExecutionCallback::target = createTreeBuilderCallback();
  ExecutionCallbackPtr pthis((DelayToUserInterfaceExecutionCallback* )this);

  if (context)
  {
    this->context = context;
    context->appendCallback(pthis);
  }

  setRootItem(new ExecutionTraceTreeViewNode(this, trace->getRootNode(), 0));
  getRootItem()->setOpen(true);
  setRootItemVisible(false);
  setColour(backgroundColourId, Colours::white);
  setMultiSelectEnabled(true);
}

ExecutionTraceTreeView::~ExecutionTraceTreeView()
{
  if (context)
  {
    ExecutionCallbackPtr pthis((DelayToUserInterfaceExecutionCallback* )this);
    context->removeCallback(pthis);
    context = ExecutionContextPtr();
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

class TabbedExecutionTraceResultsSelectorComponent : public TabbedVariableSelectorComponent
{
public:
  TabbedExecutionTraceResultsSelectorComponent(const PairPtr& pair)
    : TabbedVariableSelectorComponent(pair)
  {
    table = pair->getSecond().getObjectAndCast<Container>();
    if (table)
    {
      addTab(T("Curves"), Colours::white);
      addTab(T("Table"), Colours::white);
    }

    if (pair->getFirst().isObject())
    {
      results = pair->getFirst().getObject();
      if (results)
        addTab(T("Results"), Colours::white);
    }
  }

  virtual Variable getSubVariable(const Variable& variable, const String& tabName) const
  {
    if (tabName == T("Results"))
      return results;
    else
      return table;
  }

  virtual Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& tabName)
  {
    if (tabName == T("Curves"))
      return new ContainerCurveEditor(context, table, new ContainerCurveEditorConfiguration(table->getElementsType()));
    else if (tabName == T("Table"))
      return userInterfaceManager().createContainerTableListBox(context, table);
    else if (tabName == T("Results"))
      return userInterfaceManager().createVariableTreeView(context, variable, tabName, true, true, false, false);
    else
      return NULL;
  }

protected:
  ContainerPtr table;
  ObjectPtr results;
};

juce::Component* ExecutionTraceTreeView::createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
{
  if (!variable.exists())
    return NULL;
  if (variable.isObject())
  {
    PairPtr pair = variable.dynamicCast<Pair>();
    if (pair)
      return new TabbedExecutionTraceResultsSelectorComponent(pair);

    ContainerPtr container = variable.dynamicCast<Container>();
    if (container && !container->getElementsType()->isNamedType())
      return userInterfaceManager().createContainerTableListBox(context, container);
    else
      return userInterfaceManager().createVariableTreeView(context, variable, name, true, true, false, false);
  }
  return NULL;
}

void ExecutionTraceTreeView::timerCallback()
{
  if (!context)
    context = &defaultExecutionContext();
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
          bool hasResults = trace->getResults().size() > 0;//(size_t)(trace->getReturnValue().exists() ? 1 : 0);
          bool hasSubItems = trace->getNumSubItems() > 0;
          if (!hasResults && !hasSubItems)
            continue;
          
          ObjectPtr results;
          ContainerPtr table;
          if (hasResults)
            results = trace->getResultsObject(*context);

          if (hasSubItems)
          {
            table = trace->getChildrenResultsTable(*context);
            if (table && table->getElementsType()->getNumMemberVariables() <= 1)
              table = ContainerPtr(); // do not display tables that have only one column
          }

          if (results || table)
            selectedVariables.push_back(new Pair(results, table));
        
          if (!selectionName.isEmpty())
            selectionName += T(", ");
          selectionName += trace->toString();
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
  {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 3;}

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
    if (!traceNode)
    {
      // tmp; debug
      trace->findNode(stack);
    }
    ExecutionTraceTreeViewNode* node = tree->getNodeFromStack(stack);
    if (node)
      // node is created, fill tree and trace dynamically 
      return new MakeTraceAndFillTreeThreadExecutionCallback(tree, trace, traceNode, node);
    else
      // node is hidden/not created, fill only trace
      return new MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime());
  }

protected:
  ExecutionTraceTreeView* tree;
};

ExecutionCallbackPtr ExecutionTraceTreeView::createTreeBuilderCallback()
  {return new ExecutionTraceTreeViewBuilderExecutionCallback(this);}

juce::Component* ExecutionTrace::createComponent() const
  {return new ExecutionTraceTreeView(refCountedPointerFromThis(this));}
