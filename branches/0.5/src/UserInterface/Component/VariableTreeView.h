/*-----------------------------------------.---------------------------------.
| Filename: VariableTreeView.h             | Variable Tree component         |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_TREE_H_
# define EXPLORER_COMPONENTS_VARIABLE_TREE_H_

# include <lbcpp/UserInterface/VariableSelector.h>
# include <lbcpp/UserInterface/SimpleTreeViewItem.h>
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>

class VariableTreeViewItem;

namespace lbcpp
{

struct VariableTreeOptions
{
  VariableTreeOptions(bool showTypes = true, bool showShortSummaries = true, bool showMissingVariables = false, bool makeRootNodeVisible = true)
    : showTypes(showTypes), showShortSummaries(showShortSummaries), showMissingVariables(showMissingVariables), makeRootNodeVisible(makeRootNodeVisible) {}

  bool showTypes;
  bool showShortSummaries;
  bool showMissingVariables;
  bool makeRootNodeVisible;
};

class VariableTreeView : public juce::TreeView, public VariableSelector, public juce::Timer, public ComponentWithPreferedSize
{
public:
  VariableTreeView(const Variable& variable, const String& name, const VariableTreeOptions& options = VariableTreeOptions());
  virtual ~VariableTreeView();

  virtual bool keyPressed(const juce::KeyPress& key);

  void clearTree();
  void buildTree();

  virtual void paint(juce::Graphics& g);
  virtual void timerCallback();
  void invalidateSelection();

  virtual int getDefaultWidth() const;

  juce_UseDebuggingNewOperator

protected:
  Variable variable;
  String name;
  VariableTreeOptions options;
  VariableTreeViewItem* root;
  bool isSelectionUpToDate;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_TREE_H_
