/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeViewItem.cpp | Execution Trace TreeView Item   |
| Author  : Francis Maes                   |                                 |
| Started : 18/01/2011 11:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ExecutionTraceTreeViewItem.h"
#include "ExecutionTraceTreeView.h"
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/Data/Table.h>
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
  g.drawText(progression->toString(), x, 0, width, height, Justification::centred, true);

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
    g.drawText(lines[i], labelX, 20 * i, textWidth, 20, Justification::centredLeft, true);
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
      g.drawText(text, w, 0, timeColumnWidth, height, Justification::centredRight, false);
    }

    ExecutionTraceNodePtr workUnitTrace = trace.dynamicCast<ExecutionTraceNode>();
    if (workUnitTrace)
    {
      double timeLength = workUnitTrace->getTimeLength();
      string timeLengthString = timeLength ? ObjectPtr(new Time(timeLength))->toShortString() : T("...");
      g.drawText(timeLengthString, w + timeColumnWidth, 0, timeColumnWidth, height, Justification::centredRight, false);
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
