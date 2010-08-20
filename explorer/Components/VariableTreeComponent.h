/*-----------------------------------------.---------------------------------.
| Filename: VariableTreeComponent.h        | Variable Tree component         |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_TREE_H_
# define EXPLORER_COMPONENTS_VARIABLE_TREE_H_

# include "common.h"
# include "../Utilities/VariableSelector.h"
# include "../Utilities/SimpleTreeViewItem.h"
# include "../Utilities/ComponentWithPreferedSize.h"

class VariableTreeViewItem;

namespace lbcpp
{

struct VariableTreeOptions
{
  VariableTreeOptions(bool showTypes = true, bool showShortSummaries = true, bool showMissingVariables = true)
    : showTypes(showTypes), showShortSummaries(showShortSummaries) {}

  bool showTypes;
  bool showShortSummaries;
  bool showMissingVariables;
};

class VariableTreeComponent : public juce::TreeView, public VariableSelector, public juce::Timer, public ComponentWithPreferedSize
{
public:
  VariableTreeComponent(const Variable& variable, const String& name, const VariableTreeOptions& options = VariableTreeOptions());
  virtual ~VariableTreeComponent();

  virtual bool keyPressed(const juce::KeyPress& key);

  void clearTree();
  void buildTree();

  virtual void paint(Graphics& g);
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
