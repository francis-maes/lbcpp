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

class ObjectTreeComponent : public juce::TreeView, public ObjectSelector, public juce::Timer
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
      std::vector<ObjectPtr> selectedObjects;
      selectedObjects.reserve(getNumSelectedItems());
      for (int i = 0; i < getNumSelectedItems(); ++i)
      {
        ObjectTreeViewItem* item = dynamic_cast<ObjectTreeViewItem* >(getSelectedItem(i));
        if (item && item->getObject())
          selectedObjects.push_back(item->getObject());
      }
      sendSelectionChanged(selectedObjects);
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
  
}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_TABLE_H_
