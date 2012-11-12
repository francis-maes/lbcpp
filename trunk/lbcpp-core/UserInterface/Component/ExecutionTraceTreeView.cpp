/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.cpp     | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ExecutionTraceTreeView.h"
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/UserInterface/Plot.h>
#include <lbcpp/Core/Library.h>
#include <lbcpp/Data/Table.h>
#include <lbcpp/library.h>
using namespace lbcpp;
using juce::Graphics;
using juce::Colour;

/*
** ExecutionTraceTreeView
*/
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace, const String& name, ExecutionContextPtr context)
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

class TabbedExecutionTraceResultsSelectorComponent : public ObjectSelectorTabbedButtonBar
{
public:
  TabbedExecutionTraceResultsSelectorComponent(const PairPtr& pair)
    : ObjectSelectorTabbedButtonBar(pair)
  {
    table = pair->getSecond().staticCast<Table>();
    if (table)
    {
      addTab(T("Curves"), Colours::white);
      addTab(T("Table"), Colours::white);
    }

    results = pair->getFirst();
    if (results)
    {
      for (size_t i = 0; i < results->getNumVariables(); ++i)
        if (lbcpp::getTopLevelLibrary()->hasUIComponent(results->getVariableType(i)))
          addTab(results->getVariableName(i), Colours::lightgrey);
      addTab(T("Results"), Colours::white);
    }
  }

  virtual ObjectPtr getTabSubObject(const ObjectPtr& object, const String& tabName) const
  {
    if (tabName == T("Results"))
      return results;
    else if (tabName == T("Curves"))
      return new Plot(table);
    else if (tabName == T("Table"))
      return table;
    else
    {
      jassert(results);
      int index = results->getClass()->findMemberVariable(tabName);
      jassert(index >= 0);
      return results->getVariable(index);
    }
  }

  virtual Component* createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const String& tabName)
  {
    if (tabName == T("Table"))
      return lbcpp::getTopLevelLibrary()->createUIComponentIfExists(context, table, "Table");
    else if (tabName == T("Results"))
      return userInterfaceManager().createObjectTreeView(context, object, tabName, true, true, false, false);
    else
      return lbcpp::getTopLevelLibrary()->createUIComponentIfExists(context, getTabSubObject(object, tabName), tabName);
  }

protected:
  TablePtr table;
  ObjectPtr results;
};

juce::Component* ExecutionTraceTreeView::createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const String& name)
{
  if (!object)
    return NULL;
  PairPtr pair = object.dynamicCast<Pair>();
  if (pair)
    return new TabbedExecutionTraceResultsSelectorComponent(pair);
  return userInterfaceManager().createObjectTreeView(context, object, name, true, true, false, false);
}

void ExecutionTraceTreeView::timerCallback()
{
  if (!context)
    context = &defaultExecutionContext();
  DelayToUserInterfaceExecutionCallback::timerCallback();
  if (!isSelectionUpToDate)
  {
    std::vector<ObjectPtr> selectedObjects;
    selectedObjects.reserve(getNumSelectedItems());
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
          
          VectorPtr results;
          TablePtr table;
          if (hasResults)
            results = trace->getResultsVector(*context);

          if (hasSubItems)
          {
            table = trace->getChildrenResultsTable(*context);
            if (table && table->getNumColumns() == 1)
              table = TablePtr(); // do not display tables that have only one column
          }

          if (results || table)
            selectedObjects.push_back(new Pair(results, table));
        
          if (!selectionName.isEmpty())
            selectionName += T(", ");
          selectionName += trace->toString();
        }
      }
    }
    sendSelectionChanged(selectedObjects, selectionName);
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
    : MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime()), tree(tree), stack(1, node) {thisClass = executionCallbackClass;}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {
    MakeTraceThreadExecutionCallback::preExecutionCallback(stack, description, workUnit);
    jassert(this->stack.size());
    ExecutionTraceTreeViewNode* node = this->stack.back();
    ExecutionTraceTreeViewNode* newNode = NULL;
    if (node && node->hasBeenOpenedOnce())
      newNode = dynamic_cast<ExecutionTraceTreeViewNode* >(node->getSubItem(node->getNumSubItems() - 1));
    this->stack.push_back(newNode);
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const ObjectPtr& result)
  {
    jassert(this->stack.size());
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
    jassert(stack.size());
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
    : tree(tree) {thisClass = executionCallbackClass;}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId)
  {
    ExecutionTracePtr trace = tree->getTrace();
    ExecutionTraceNodePtr traceNode = trace->findNode(stack);
    jassert(traceNode);
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
