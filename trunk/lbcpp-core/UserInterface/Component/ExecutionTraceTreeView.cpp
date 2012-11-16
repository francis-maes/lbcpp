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
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace, const string& name, ExecutionContextPtr context)
  : GenericTreeView(trace, name)
{
  if (context)
  {
    this->context = context;
    notificationQueue = new NotificationQueue();
    targetCallback = createTreeBuilderCallback();
    ExecutionCallback::setStaticAllocationFlag();
    ExecutionCallbackPtr pthis((ExecutionCallback* )this);
    context->appendCallback(pthis);
  }

  buildTree();
  setRootItemVisible(false);
}

ExecutionTraceTreeView::~ExecutionTraceTreeView()
{
  if (context)
  {
    ExecutionCallbackPtr pthis((ExecutionCallback* )this);
    context->removeCallback(pthis);
    context = ExecutionContextPtr();
  }
}

GenericTreeViewItem* ExecutionTraceTreeView::createItem(const ObjectPtr& object, const string& name)
{
  ExecutionTraceItemPtr item = object.dynamicCast<ExecutionTraceItem>();
  if (item)
    return ExecutionTraceTreeViewItem::create(this, object.staticCast<ExecutionTraceItem>());
  else
  {
    jassert(object.isInstanceOf<ExecutionTrace>());
    return new ExecutionTraceTreeViewNode(this, object.staticCast<ExecutionTrace>()->getRootNode());
  }
}

bool ExecutionTraceTreeView::mightHaveSubObjects(const ObjectPtr& object)
{
  ExecutionTraceNodePtr node = object.dynamicCast<ExecutionTraceNode>();
  return node && node->getNumSubItems() > 0;
}

std::vector< std::pair<string, ObjectPtr> > ExecutionTraceTreeView::getSubObjects(const ObjectPtr& object)
{
  ExecutionTraceNodePtr node = object.dynamicCast<ExecutionTraceNode>();
  if (node)
  {
    std::vector<ExecutionTraceItemPtr> subItems = node->getSubItems();
    std::vector< std::pair<string, ObjectPtr> > res(subItems.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = std::make_pair(subItems[i]->toShortString(), subItems[i]);
    return res;
  }
  else
    return std::vector< std::pair<string, ObjectPtr> >();
}
  

ExecutionTraceTreeViewNode* ExecutionTraceTreeView::getNodeFromStack(const ExecutionStackPtr& stack) const
{
  ExecutionTraceTreeViewNode* item = (ExecutionTraceTreeViewNode* )getRootItem();
  size_t n = stack->getDepth();
  for (size_t i = 0; i < n; ++i)
  {
    const std::pair<string, WorkUnitPtr>& entry = stack->getEntry(i);
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
      plot = new Plot(table);
      if (plot->getNumPlotVariables() >= 2)
        addTab(T("Plot"), Colours::white);
      if (table->getNumColumns() >= 2 && table->getNumRows() > 1)
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

  virtual ObjectPtr getTabSubObject(const ObjectPtr& object, const string& tabName) const
  {
    if (tabName == T("Results"))
      return results;
    else if (tabName == T("Plot"))
      return plot;
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

  virtual Component* createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const string& tabName)
  {
    if (tabName == T("Results"))
      return userInterfaceManager().createObjectTreeView(context, object, tabName, false);
    else
      return lbcpp::getTopLevelLibrary()->createUIComponentIfExists(context, getTabSubObject(object, tabName), tabName);
  }

protected:
  TablePtr table;
  ObjectPtr results;
  PlotPtr plot;
};

juce::Component* ExecutionTraceTreeView::createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const string& name)
{
  if (!object)
    return NULL;
  PairPtr pair = object.dynamicCast<Pair>();
  if (pair)
    return new TabbedExecutionTraceResultsSelectorComponent(pair);
  return userInterfaceManager().createObjectTreeView(context, object, name, false);
}

void ExecutionTraceTreeView::timerCallback()
{
  if (!context)
    context = &defaultExecutionContext();
  notificationQueue->flush(targetCallback);
  GenericTreeView::timerCallback();
}

#include "../../Execution/Callback/MakeTraceExecutionCallback.h"

class MakeTraceAndFillTreeThreadExecutionCallback : public MakeTraceThreadExecutionCallback
{
public:
  MakeTraceAndFillTreeThreadExecutionCallback(ExecutionTraceTreeView* tree, ExecutionTracePtr trace, ExecutionTraceNodePtr traceNode, ExecutionTraceTreeViewNode* node)
    : MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime()), tree(tree), stack(1, node) {thisClass = executionCallbackClass;}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit)
  {
    MakeTraceThreadExecutionCallback::preExecutionCallback(stack, description, workUnit);
    jassert(this->stack.size());
    ExecutionTraceTreeViewNode* node = this->stack.back();
    ExecutionTraceTreeViewNode* newNode = NULL;
    if (node && node->hasBeenOpenedOnce())
      newNode = dynamic_cast<ExecutionTraceTreeViewNode* >(node->getSubItem(node->getNumSubItems() - 1));
    this->stack.push_back(newNode);
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit, const ObjectPtr& result)
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
      ExecutionTraceTreeViewItem* newItem = ExecutionTraceTreeViewItem::create(tree, item);
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
