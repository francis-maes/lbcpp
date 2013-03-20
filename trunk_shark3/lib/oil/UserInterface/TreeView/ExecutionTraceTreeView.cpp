/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeView.cpp     | Execution Trace TreeView        |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ExecutionTraceTreeView.h"
#include <oil/Execution/ExecutionStack.h>
#include <oil/UserInterface/Plot.h>
#include <oil/Core/Library.h>
#include <oil/Core/Table.h>
#include <oil/library.h>
using namespace lbcpp;
using juce::Graphics;
using juce::Colour;

/*
** ExecutionTraceTreeViewItem
*/
ExecutionTraceTreeViewItem::ExecutionTraceTreeViewItem(GenericTreeView* owner, const ExecutionTraceItemPtr& trace)
  : GenericTreeViewItem(owner, trace, trace->toShortString())
{
  ExecutionTraceNodePtr node = trace.dynamicCast<ExecutionTraceNode>();
  if (node)
  {
    WorkUnitPtr workUnit = node->getWorkUnit();
    if (workUnit)
    {
      // online
      CompositeWorkUnitPtr compositeWorkUnit = node->getWorkUnit().dynamicCast<CompositeWorkUnit>();
      setOpen((!compositeWorkUnit || compositeWorkUnit->getNumWorkUnits() <= 10));
    }
    else
      setOpen(node->getNumSubItems() < 10 && node->getTimeLength() > 1); // open nodes that took more than 1 seconds and that have less than 10 childs
  }
}

int ExecutionTraceTreeViewItem::getMaximumColumnWidth(size_t columnNumber) const
{
  if (columnNumber == 1)
    return 120;
  if (columnNumber == 2)
    return 80;
  else
    return 200;
}

void ExecutionTraceTreeViewItem::paintColumn(Graphics& g, size_t columnNumber, ObjectPtr data, int x, int y, int width, int height) const
{
  if (columnNumber == 0)
  {
    ProgressionStatePtr progression = data.staticCast<ProgressionState>();

    juce::GradientBrush brush(Colour(200, 220, 240), (float)x, -(float)width / 3.f, Colours::white, (float)width, (float)width, true);

    g.setBrush(&brush);
    if (progression->isBounded())
      g.fillRect(x, 0, (int)(width * progression->getNormalizedValue() + 0.5), height);
    else
    {
      double p = (progression->getValue() / 1000.0);
      p -= (int)p;
      jassert(p >= 0.0 && p <= 1.0);
      g.fillRect(x + (int)(width * p + 0.5) - 3, 0, 6, height);
    }
    g.setBrush(NULL);

    g.setColour(Colours::black);
    g.setFont(10);
    g.drawText(progression->toString(), x, y, width, height, juce::Justification::centred, true);

    g.setColour(Colour(180, 180, 180));
    g.drawRect(x, 0, width, height, 1);
  }
  else
    return GenericTreeViewItem::paintColumn(g, columnNumber, data, x, y, width, height);
}

ObjectPtr ExecutionTraceTreeViewItem::getTargetObject(ExecutionContext& context) const
{
  ExecutionTraceNodePtr trace = object.dynamicCast<ExecutionTraceNode>();
  if (!trace)
    return ObjectPtr();
  
  bool hasResults = trace->getResults().size() > 0;//(size_t)(trace->getReturnValue().exists() ? 1 : 0);
  bool hasSubItems = trace->getNumSubItems() > 0;
  if (!hasResults && !hasSubItems)
    return ObjectPtr();
          
  VectorPtr results;
  TablePtr table;
  if (hasResults)
    results = trace->getResultsVector(context);

  if (hasSubItems)
  {
    table = trace->getChildrenResultsTable(context);
    if (table && table->getNumColumns() == 1)
      table = TablePtr(); // do not display tables that have only one column
  }

  if (results || table)
    return new Pair(results, table);
  else
    return ObjectPtr();
}

/*
** ExecutionTraceTreeView
*/
ExecutionTraceTreeView::ExecutionTraceTreeView(ExecutionTracePtr trace, const string& name, ExecutionContextPtr context)
  : GenericTreeView(trace, name)
{
  ExecutionCallback::setStaticAllocationFlag();
  if (context)
  {
    this->context = context;
    notificationQueue = new NotificationQueue();
    targetCallback = createTreeBuilderCallback();
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
  ExecutionTracePtr trace = object.dynamicCast<ExecutionTrace>();
  ExecutionTraceItemPtr item = trace ? (ExecutionTraceItemPtr)trace->getRootNode() : object.staticCast<ExecutionTraceItem>();
  return new ExecutionTraceTreeViewItem(this, item);
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
  

size_t ExecutionTraceTreeView::getNumDataColumns()
  {return 3;}

std::vector<ObjectPtr> ExecutionTraceTreeView::getObjectData(const ObjectPtr& object)
{
  std::vector<ObjectPtr> res(3);
  ExecutionTraceNodePtr node = object.dynamicCast<ExecutionTraceNode>();
  if (node)
  {
    res[0] = node->getProgression();
    res[1] = node->getReturnValue();
    res[2] = new Time(node->getTimeLength());
  }
  return res;
}

ExecutionTraceTreeViewItem* ExecutionTraceTreeView::getNodeFromStack(const ExecutionStackPtr& stack) const
{
  ExecutionTraceTreeViewItem* item = (ExecutionTraceTreeViewItem* )getRootItem();
  size_t n = stack->getDepth();
  for (size_t i = 0; i < n; ++i)
  {
    const std::pair<string, WorkUnitPtr>& entry = stack->getEntry(i);
    bool ok = false;
    for (int i = 0; i < item->getNumSubItems(); ++i)
    {
      ExecutionTraceTreeViewItem* subItem = static_cast<ExecutionTraceTreeViewItem* >(item->getSubItem(i));
      ExecutionTraceNodePtr traceNode = subItem->getTraceNode();
      if (traceNode)
      {
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
        addTab(JUCE_T("Plot"), Colours::white);
      if (table->getNumColumns() >= 2 && table->getNumRows() > 1)
        addTab(JUCE_T("Table"), Colours::white);
    }

    results = pair->getFirst();
    if (results)
    {
      for (size_t i = 0; i < results->getNumVariables(); ++i)
        if (lbcpp::getTopLevelLibrary()->hasUIComponent(results->getVariableType(i)))
          addTab(results->getVariableName(i), Colours::lightgrey);
      addTab(JUCE_T("Results"), Colours::white);
    }
  }

  virtual ObjectPtr getTabSubObject(const ObjectPtr& object, const string& tabName) const
  {
    if (tabName == JUCE_T("Results"))
      return results;
    else if (tabName == JUCE_T("Plot"))
      return plot;
    else if (tabName == JUCE_T("Table"))
      return table;
    else
    {
      jassert(results);
      int index = results->getClass()->findMemberVariable(tabName);
      jassert(index >= 0);
      return results->getVariable(index);
    }
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
  if (notificationQueue)
    notificationQueue->flush(targetCallback);
  GenericTreeView::timerCallback();
}

#include "../../Execution/Callback/MakeTraceExecutionCallback.h"

class MakeTraceAndFillTreeThreadExecutionCallback : public MakeTraceThreadExecutionCallback
{
public:
  MakeTraceAndFillTreeThreadExecutionCallback(ExecutionTraceTreeView* tree, ExecutionTracePtr trace, ExecutionTraceNodePtr traceNode, ExecutionTraceTreeViewItem* node)
    : MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime()), tree(tree), stack(1, node) {thisClass = executionCallbackClass;}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit)
  {
    MakeTraceThreadExecutionCallback::preExecutionCallback(stack, description, workUnit);
    jassert(this->stack.size());
    ExecutionTraceTreeViewItem* node = this->stack.back();
    ExecutionTraceTreeViewItem* newNode = NULL;
    if (node && node->hasBeenOpenedOnce())
      newNode = dynamic_cast<ExecutionTraceTreeViewItem* >(node->getSubItem(node->getNumSubItems() - 1));
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
  std::vector<ExecutionTraceTreeViewItem* > stack;

  virtual void appendTraceItem(ExecutionTraceItemPtr item)
  {
    jassert(stack.size());
    MakeTraceThreadExecutionCallback::appendTraceItem(item);
    ExecutionTraceTreeViewItem* parent = stack.back();
    if (parent && parent->hasBeenOpenedOnce())
    {
      ExecutionTraceTreeViewItem* newItem = new ExecutionTraceTreeViewItem(tree, item);
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
    ExecutionTraceTreeViewItem* node = tree->getNodeFromStack(stack);
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
