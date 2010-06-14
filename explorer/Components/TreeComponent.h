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
# include "../Utilities/ObjectSelector.h"
# include "../Utilities/SimpleTreeViewItem.h"

namespace lbcpp
{

class ObjectTreeViewItem : public SimpleTreeViewItem
{
public:
  ObjectTreeViewItem(ObjectPtr object, const String& name, ObjectSelector& selector)
    : SimpleTreeViewItem(name, NULL, true),
      object(object), component(NULL), selector(selector)
  {
    object->getChildrenObjects(subObjects);
    mightContainSubItemsFlag = subObjects.size() > 0;
  }

  virtual void itemSelectionChanged(bool isNowSelected)
    {if (isNowSelected) selector.sendObjectSelected(object);}

  virtual void createSubItems()
  {
    for (size_t i = 0; i < subObjects.size(); ++i)
      addSubItem(new ObjectTreeViewItem(subObjects[i].second, subObjects[i].first, selector));
  }
  
  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  Component* component;
  std::vector< std::pair<String, ObjectPtr> > subObjects;
  ObjectSelector& selector;
};

class ObjectTreeComponent : public juce::TreeView, public ObjectSelector
{
public:
  ObjectTreeComponent(ObjectPtr object, const String& name) 
    : object(object), name(name), root(NULL)
  {
    setRootItemVisible(true);
    setWantsKeyboardFocus(true);
    buildTree();
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
    root = new ObjectTreeViewItem(object, name, *this);
    setRootItem(root);
    root->setOpen(true);
  }

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::white);
    juce::TreeView::paint(g);
  }

  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  String name;
  ObjectTreeViewItem* root;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_TABLE_H_
