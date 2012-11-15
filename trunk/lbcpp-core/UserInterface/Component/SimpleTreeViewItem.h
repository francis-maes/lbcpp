/*-----------------------------------------.---------------------------------.
| Filename: SimpleTreeViewItem.h           | Base class for tree view items  |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_SIMPLE_TREE_VIEW_ITEM_H_
# define LBCPP_USER_INTERFACE_SIMPLE_TREE_VIEW_ITEM_H_

# include <lbcpp/UserInterface/UserInterfaceManager.h>

namespace lbcpp
{

class SimpleTreeViewItem : public juce::TreeViewItem
{
public:
  SimpleTreeViewItem(const string& uniqueName, juce::Image* iconToUse = 0, bool mightContainSubItems = false)
    : uniqueName(uniqueName), iconToUse(iconToUse), mightContainSubItemsFlag(mightContainSubItems), hasBeenOpened(false) {}

  SimpleTreeViewItem(const string& uniqueName, const string& iconToUse, bool mightContainSubItems = false)
    : uniqueName(uniqueName), iconToUse(iconToUse.isEmpty() ? NULL : userInterfaceManager().getImage(iconToUse, 18, 18)),
      mightContainSubItemsFlag(mightContainSubItems), hasBeenOpened(false) {}
     
  virtual bool mightContainSubItems()
    {return mightContainSubItemsFlag;}

  void setUniqueName(const string& name)
    {this->uniqueName = name;}

  virtual const string getUniqueName() const
    {return uniqueName;}
    
  enum {defaultIconSize = 18};

  virtual void paintItem(juce::Graphics& g, int width, int height)
  {
    if (isSelected())
      g.fillAll(juce::Colours::lightgrey);
    g.setColour(juce::Colours::black);
    int x1 = 0;
    if (iconToUse)
    {
      g.drawImageAt(iconToUse, 0, (height - iconToUse->getHeight()) / 2);
      x1 += iconToUse->getWidth() + 5;
    }
    g.drawText(getUniqueName(), x1, 0, width - x1, height, juce::Justification::centredLeft, true);
  }

  virtual void createSubItems() {}

  virtual void itemOpennessChanged(bool isNowOpen)
  {
    if (isNowOpen && !hasBeenOpened)
    {
      hasBeenOpened = true;
      createSubItems();
    }
  }
  
  void setIcon(const string& name)
    {iconToUse = userInterfaceManager().getImage(name, 18, 18);}

  bool hasBeenOpenedOnce() const
    {return hasBeenOpened;}

protected:
  string uniqueName;
  juce::Image* iconToUse;
  bool mightContainSubItemsFlag;
  bool hasBeenOpened;
};

class GenericTreeView : public juce::TreeView, public ObjectSelector, public ComponentWithPreferedSize
{
public:
  GenericTreeView(ObjectPtr object, const string& name)
    : object(object), name(name)
  {
    setColour(backgroundColourId, Colours::white);
  }

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() / 4;}

  juce_UseDebuggingNewOperator

protected:
  ObjectPtr object;
  string name;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_SIMPLE_TREE_VIEW_ITEM_H_
