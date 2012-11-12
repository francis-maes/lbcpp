/*-----------------------------------------.---------------------------------.
| Filename: ObjectTreeView.h               | Object Tree component           |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_TREE_H_
# define EXPLORER_COMPONENTS_VARIABLE_TREE_H_

# include <lbcpp/UserInterface/ObjectComponent.h>
# include "SimpleTreeViewItem.h"

class ObjectTreeViewItem;

namespace lbcpp
{

struct ObjectTreeOptions
{
  ObjectTreeOptions(bool showTypes = true, bool showShortSummaries = true, bool showMissingVariables = false, bool makeRootNodeVisible = true)
    : showTypes(showTypes), showShortSummaries(showShortSummaries), showMissingVariables(showMissingVariables), makeRootNodeVisible(makeRootNodeVisible) {}

  bool showTypes;
  bool showShortSummaries;
  bool showMissingVariables;
  bool makeRootNodeVisible;
};

class ObjectTreeView : public juce::TreeView, public ObjectSelector, public juce::Timer, public ComponentWithPreferedSize
{
public:
  ObjectTreeView(const ObjectPtr& object, const string& name, const ObjectTreeOptions& options = ObjectTreeOptions());
  virtual ~ObjectTreeView();

  virtual bool keyPressed(const juce::KeyPress& key);

  void clearTree();
  void buildTree();

  virtual void paint(juce::Graphics& g);
  virtual void timerCallback();
  void invalidateSelection();

  virtual int getDefaultWidth() const;

  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  string name;
  ObjectTreeOptions options;
  ObjectTreeViewItem* root;
  bool isSelectionUpToDate;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_TREE_H_
