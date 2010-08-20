/*-----------------------------------------.---------------------------------.
| Filename: TreeComponent.h                | Base class for Tree components  |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_TREE_H_
# define EXPLORER_COMPONENTS_TREE_H_

# include "common.h"
# include "../Utilities/VariableSelector.h"
# include "../Utilities/SimpleTreeViewItem.h"

namespace lbcpp
{

class ObjectTreeViewItem : public SimpleTreeViewItem
{
public:
  ObjectTreeViewItem(ObjectPtr object, const String& name)
    : SimpleTreeViewItem(name, NULL, true),
      object(object), component(NULL)
  {
    if (object)
      object->getChildrenObjects(subObjects);
    mightContainSubItemsFlag = subObjects.size() > 0;
  }

  virtual void itemSelectionChanged(bool isNowSelected);
  
  virtual void createSubItems()
  {
    for (size_t i = 0; i < subObjects.size(); ++i)
      addSubItem(new ObjectTreeViewItem(subObjects[i].second, subObjects[i].first));
  }

  ObjectPtr getObject() const
    {return object;}
  
  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  Component* component;
  std::vector< std::pair<String, ObjectPtr> > subObjects;
};

class ObjectTreeComponent : public juce::TreeView, public VariableSelector, public juce::Timer
{
public:
  ObjectTreeComponent(ObjectPtr object, const String& name) 
    : object(object), name(name), root(NULL), isSelectionUpToDate(false)
  {
    setRootItemVisible(true);
    setWantsKeyboardFocus(true);
    setMultiSelectEnabled(true);
    buildTree();
    root->setSelected(true, true);
    startTimer(100);  
  }

  virtual ~ObjectTreeComponent()
    {clearTree();}

  virtual bool keyPressed(const juce::KeyPress& key)
  {
    if (key.getKeyCode() == juce::KeyPress::F5Key)
    {
      clearTree(), buildTree();
      return true;
    }
    return juce::TreeView::keyPressed(key);
  }
  
  void clearTree()
  {
    if (root)
    {
      deleteRootItem();
      root = NULL;
    }    
  }

  void buildTree()
  {
    root = new ObjectTreeViewItem(object, name);
    setRootItem(root);
    root->setOpen(true);
  }

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::white);
    juce::TreeView::paint(g);
  }
  
  virtual void timerCallback()
  {
    if (!isSelectionUpToDate)
    {
      std::vector<Variable> selectedVariables;
      selectedVariables.reserve(getNumSelectedItems());
      for (int i = 0; i < getNumSelectedItems(); ++i)
      {
        ObjectTreeViewItem* item = dynamic_cast<ObjectTreeViewItem* >(getSelectedItem(i));
        if (item && item->getObject())
          selectedVariables.push_back(item->getObject());
      }
      sendSelectionChanged(selectedVariables);
      isSelectionUpToDate = true;
    }
  }
  
  void invalidateSelection()
    {isSelectionUpToDate = false;}

  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  String name;
  ObjectTreeViewItem* root;
  bool isSelectionUpToDate;
};

inline void ObjectTreeViewItem::itemSelectionChanged(bool isNowSelected)
{
  ObjectTreeComponent* owner = dynamic_cast<ObjectTreeComponent* >(getOwnerView());
  jassert(owner);
  owner->invalidateSelection();
}

//////////////////////////////////////////////////////////////////////////////

struct VariableTreeOptions
{
  VariableTreeOptions(bool showTypes = true, bool showShortSummaries = true)
    : showTypes(showTypes), showShortSummaries(showShortSummaries) {}

  bool showTypes;
  bool showShortSummaries;
};

class VariableTreeViewItem : public SimpleTreeViewItem
{
public:
  VariableTreeViewItem(const String& name, const Variable& variable, const VariableTreeOptions& options)
    : SimpleTreeViewItem(name, NULL, true), 
      variable(variable), options(options), typeName(variable.getTypeName()), component(NULL)
  {
    shortSummary = variable.getShortSummary();

    size_t n = variable.size();
    subVariables.resize(n);
    for (size_t i = 0; i < n; ++i)
      subVariables[i] = std::make_pair(variable.getVariableName(i), variable[i]);
    mightContainSubItemsFlag = !subVariables.empty();
  }

  virtual void itemSelectionChanged(bool isNowSelected);
  
  virtual void createSubItems()
  {
    for (size_t i = 0; i < subVariables.size(); ++i)
      addSubItem(new VariableTreeViewItem(subVariables[i].first, subVariables[i].second, options));
  }

  Variable getVariable() const
    {return variable;}
  
  virtual void paintItem(Graphics& g, int width, int height)
  {
    if (isSelected())
      g.fillAll(Colours::lightgrey);
    g.setColour(Colours::black);
    int x1 = 0;
    if (iconToUse)
    {
      g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);
      x1 += iconToUse->getWidth() + 5;
    }

    int numFields = 1;
    if (options.showTypes) ++numFields;
    if (options.showShortSummaries) ++numFields;

    int typeAndNameLength;
    enum {wantedLength = 300};
    int remainingWidth = width - x1;
    if (remainingWidth >= numFields * wantedLength)
      typeAndNameLength = wantedLength;
    else
      typeAndNameLength = remainingWidth / numFields;

    g.setFont(Font(12, Font::bold));
    g.drawText(getUniqueName(), x1, 0, typeAndNameLength - 5, height, Justification::centredLeft, true);
    x1 += typeAndNameLength;
    if (options.showTypes)
    {
      g.setFont(Font(12, Font::italic));
      g.drawText(typeName, x1, 0, typeAndNameLength - 5, height, Justification::centredLeft, true);
      x1 += typeAndNameLength;
    }

    if (options.showShortSummaries && shortSummary.isNotEmpty())
    {
      g.setFont(Font(12));
      g.drawText(shortSummary, x1, 0, width - x1 - 2, height, Justification::centredLeft, true);
    }
  }

  juce_UseDebuggingNewOperator

protected:
  Variable variable;
  const VariableTreeOptions& options;
  String typeName;
  String shortSummary;
  Component* component;

  std::vector< std::pair<String, Variable> > subVariables;
};

class VariableTreeComponent : public juce::TreeView, public VariableSelector, public juce::Timer
{
public:
  VariableTreeComponent(const Variable& variable, const String& name, const VariableTreeOptions& options = VariableTreeOptions())
    : variable(variable), name(name), options(options), root(NULL), isSelectionUpToDate(false)
  {
    setRootItemVisible(true);
    setWantsKeyboardFocus(true);
    setMultiSelectEnabled(true);
    buildTree();
    root->setSelected(true, true);
    startTimer(100);  
  }

  virtual ~VariableTreeComponent()
    {clearTree();}

  virtual bool keyPressed(const juce::KeyPress& key)
  {
    if (key.getKeyCode() == juce::KeyPress::F5Key)
    {
      clearTree(), buildTree();
      return true;
    }
    return juce::TreeView::keyPressed(key);
  }
  
  void clearTree()
  {
    if (root)
    {
      deleteRootItem();
      root = NULL;
    }    
  }

  void buildTree()
  {
    root = new VariableTreeViewItem(name, variable, options);
    setRootItem(root);
    root->setOpen(true);
  }

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::white);
    juce::TreeView::paint(g);
  }
  
  virtual void timerCallback()
  {
    if (!isSelectionUpToDate)
    {
      std::vector<Variable> selectedVariables;
      selectedVariables.reserve(getNumSelectedItems());
      for (int i = 0; i < getNumSelectedItems(); ++i)
      {
        VariableTreeViewItem* item = dynamic_cast<VariableTreeViewItem* >(getSelectedItem(i));
        if (item && item->getVariable())
          selectedVariables.push_back(item->getVariable());
      }
      sendSelectionChanged(selectedVariables);
      isSelectionUpToDate = true;
    }
  }
  
  void invalidateSelection()
    {isSelectionUpToDate = false;}

  juce_UseDebuggingNewOperator

protected:
  Variable variable;
  String name;
  VariableTreeOptions options;
  VariableTreeViewItem* root;
  bool isSelectionUpToDate;
};

inline void VariableTreeViewItem::itemSelectionChanged(bool isNowSelected)
{
  VariableTreeComponent* owner = dynamic_cast<VariableTreeComponent* >(getOwnerView());
  jassert(owner);
  owner->invalidateSelection();
}

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_TABLE_H_
