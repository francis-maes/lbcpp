/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeViewItem.h   | Execution Trace TreeView Items  |
| Author  : Francis Maes                   |                                 |
| Started : 18/01/2011 11:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_ITEM_H_
# define LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_ITEM_H_

# include <lbcpp/UserInterface/ObjectComponent.h>
# include <lbcpp/Execution/ExecutionTrace.h>
# include "GenericTreeView.h"

using juce::Component;
using juce::DocumentWindow;

using juce::Image;
using juce::Colours;
using juce::Justification;

using juce::TreeView;
using juce::TreeViewItem;

namespace lbcpp
{

class ExecutionTraceTreeView;
class ExecutionTraceTreeViewItem : public GenericTreeViewItem
{
public:
  ExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace);

  static ExecutionTraceTreeViewItem* create(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace);

  enum
  {
    minWidthToDisplayTimes = 300,
    minWidthToDisplayProgression = 150,
    progressionColumnWidth = 150,
    labelX = 23,
    timeColumnWidth = 100,
  };
 
  void paintProgression(juce::Graphics& g, ProgressionStatePtr progression, int x, int width, int height);
  void paintIcon(juce::Graphics& g, int width, int height);
  void paintIconTextAndProgression(juce::Graphics& g, int width, int height);

  virtual void paintItem(juce::Graphics& g, int width, int height);
  
  virtual int getItemHeight() const
    {return 20 * numLines;}

  const ExecutionTraceItemPtr& getTrace() const
    {return object.staticCast<ExecutionTraceItem>();}

protected:
  int numLines;
};

class ExecutionTraceTreeViewNode : public ExecutionTraceTreeViewItem
{
public:
  ExecutionTraceTreeViewNode(ExecutionTraceTreeView* owner, const ExecutionTraceNodePtr& trace);

  const ExecutionTraceNodePtr& getTraceNode() const
    {return object.staticCast<ExecutionTraceNode>();}

  virtual void itemOpennessChanged(bool isNowOpen);

  virtual ObjectPtr getTargetObject(ExecutionContext& context) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_ITEM_H_
