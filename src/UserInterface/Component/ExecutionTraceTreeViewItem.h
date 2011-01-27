/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTraceTreeViewItem.h   | Execution Trace TreeView Items  |
| Author  : Francis Maes                   |                                 |
| Started : 18/01/2011 11:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_ITEM_H_
# define LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_ITEM_H_

# include <lbcpp/UserInterface/SimpleTreeViewItem.h>
# include <lbcpp/UserInterface/VariableSelector.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <lbcpp/Execution/ExecutionTrace.h>

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
class ExecutionTraceTreeViewItem : public SimpleTreeViewItem
{
public:
  ExecutionTraceTreeViewItem(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace, size_t depth);

  static ExecutionTraceTreeViewItem* create(ExecutionTraceTreeView* owner, const ExecutionTraceItemPtr& trace, size_t depth);

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

  static String formatTime(double timeInSeconds);
  
  virtual int getItemHeight() const
    {return 20 * numLines;}

  const ExecutionTraceItemPtr& getTrace() const
    {return trace;}

  virtual void itemSelectionChanged(bool isNowSelected);
  
  ExecutionTraceTreeView* getOwner() const
    {return owner;}

  size_t getDepth() const
    {return depth;}

protected:
  ExecutionTraceTreeView* owner;
  ExecutionTraceItemPtr trace;
  int numLines;
  size_t depth;
};

class ExecutionTraceTreeViewNode : public ExecutionTraceTreeViewItem
{
public:
  ExecutionTraceTreeViewNode(ExecutionTraceTreeView* owner, const ExecutionTraceNodePtr& trace, size_t depth);

  virtual bool mightContainSubItems()
    {return getTraceNode()->getNumSubItems() > 0;}

  const ExecutionTraceNodePtr& getTraceNode() const
    {return trace.staticCast<ExecutionTraceNode>();}

  virtual void createSubItems();
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_EXECUTION_TRACE_TREE_VIEW_ITEM_H_
