/*-----------------------------------------.---------------------------------.
| Filename: GenericTreeView.h              | Base classes for tree views     |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_TREE_VIEW_GENERIC_H_
# define LBCPP_USER_INTERFACE_TREE_VIEW_GENERIC_H_

# include <lbcpp/UserInterface/UserInterfaceManager.h>
# include <lbcpp/UserInterface/ObjectComponent.h>

namespace lbcpp
{

class GenericTreeView;
class GenericTreeViewItem : public juce::TreeViewItem
{
public:
  GenericTreeViewItem(GenericTreeView* owner, ObjectPtr object, const string& uniqueName);
     
  enum {defaultIconSize = 18};

  virtual bool mightContainSubItems();
  virtual const string getUniqueName() const;
  virtual void paintItem(juce::Graphics& g, int width, int height);
  virtual void itemOpennessChanged(bool isNowOpen);
  virtual void itemSelectionChanged(bool isNowSelected);
  
  void setIcon(const string& name);

  bool hasBeenOpenedOnce() const
    {return hasBeenOpened;}

  GenericTreeView* getOwner() const
    {return owner;}

  const ObjectPtr& getObject() const
    {return object;}
  
  virtual ObjectPtr getTargetObject(ExecutionContext& context) const;
  
  void createSubItems();

protected:
  GenericTreeView* owner;
  ObjectPtr object;
  string uniqueName;
  juce::Image* iconToUse;
  bool hasBeenOpened;
};

class GenericTreeView : public juce::TreeView, public ObjectSelector, public ComponentWithPreferedSize, public juce::Timer
{
public:
  GenericTreeView(ObjectPtr object, const string& name);
  ~GenericTreeView();

  virtual int getDefaultWidth() const;
  virtual void timerCallback();
  virtual bool keyPressed(const juce::KeyPress& key);

  void buildTree();
  void clearTree();
  
  void invalidateSelection()
    {isSelectionUpToDate = false;}
  
  void invalidateTree()
    {isTreeUpToDate = false;}

  // to override
  virtual GenericTreeViewItem* createItem(const ObjectPtr& object, const string& name) = 0;
  virtual bool mightHaveSubObjects(const ObjectPtr& object) = 0;
  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object) = 0;

  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  string name;

  GenericTreeViewItem* root;
  bool isSelectionUpToDate;
  bool isTreeUpToDate;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_GENERIC_H_
