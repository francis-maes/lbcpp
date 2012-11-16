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
** ExecutionTraceTreeViewItem
*/
ExecutionTraceTreeViewItem::ExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace)
  : GenericTreeViewItem(owner, trace, trace->toShortString())
{
  string str = getUniqueName();
  numLines = 1;
  for (int i = 0; i < str.length() - 1; ++i)
    if (str[i] == '\n')
      ++numLines;
}

ExecutionTraceTreeViewItem* ExecutionTraceTreeViewItem::create(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& item)
{
  ExecutionTraceNodePtr node = item.dynamicCast<ExecutionTraceNode>();
  if (node)
    return new ExecutionTraceTreeViewNode(owner, node);
  else
    return new ExecutionTraceTreeViewItem(owner, item);
}

void ExecutionTraceTreeViewItem::paintProgression(Graphics& g, ProgressionStatePtr progression, int x, int width, int height)
{
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
  g.drawText(progression->toString(), x, 0, width, height, juce::Justification::centred, true);

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

  ExecutionTraceNodePtr traceNode = getTrace().dynamicCast<ExecutionTraceNode>();
  if (traceNode && traceNode->getProgression() && width > minWidthToDisplayProgression)
  {
    textWidth -= progressionColumnWidth;
    paintProgression(g, traceNode->getProgression(), labelX + textWidth, progressionColumnWidth, height);
  }
  
  g.setColour(Colours::black);
  g.setFont(12);
  
  string str = getUniqueName();
  StringArray lines;
  lines.addTokens(str, T("\n"), NULL);
  for (int i = 0; i < lines.size(); ++i)
    g.drawText(lines[i], labelX, 20 * i, textWidth, 20, juce::Justification::centredLeft, true);
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

    ExecutionTraceItemPtr trace = getTrace();
    ExecutionTraceNodePtr node = trace.dynamicCast<ExecutionTraceNode>();
    if (node)
    {
      ObjectPtr returnValue = node->getReturnValue();
      string text = returnValue ? returnValue->toShortString() : string::empty;
      g.drawText(text, w, 0, timeColumnWidth, height, juce::Justification::centredRight, false);
    }

    ExecutionTraceNodePtr workUnitTrace = trace.dynamicCast<ExecutionTraceNode>();
    if (workUnitTrace)
    {
      double timeLength = workUnitTrace->getTimeLength();
      string timeLengthString = timeLength ? ObjectPtr(new Time(timeLength))->toShortString() : T("...");
      g.drawText(timeLengthString, w + timeColumnWidth, 0, timeColumnWidth, height, juce::Justification::centredRight, false);
    }
  }
  else
    paintIconTextAndProgression(g, width, height);
}

/*
** ExecutionTraceTreeViewNode
*/
ExecutionTraceTreeViewNode::ExecutionTraceTreeViewNode(ExecutionTraceTreeView* owner, const ExecutionTraceNodePtr& trace)
  : ExecutionTraceTreeViewItem(owner, trace)
{
  WorkUnitPtr workUnit = trace->getWorkUnit();
  if (workUnit)
  {
    // online
    CompositeWorkUnitPtr compositeWorkUnit = trace->getWorkUnit().dynamicCast<CompositeWorkUnit>();
    setOpen((!compositeWorkUnit || compositeWorkUnit->getNumWorkUnits() <= 10));
  }
  else
    setOpen(trace->getNumSubItems() < 10 && trace->getTimeLength() > 1); // open nodes that took more than 1 seconds and that have less than 10 childs
}

void ExecutionTraceTreeViewNode::itemOpennessChanged(bool isNowOpen)
{
  if (isNowOpen)
  {
    if (!hasBeenOpened)
      hasBeenOpened = true;
    if (getTraceNode()->getNumSubItems() != (size_t)getNumSubItems())
    {
      clearSubItems();
      createSubItems();
    }
  }
}

ObjectPtr ExecutionTraceTreeViewNode::getTargetObject(ExecutionContext& context) const
{
  ExecutionTraceNodePtr trace = getTraceNode();
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
