/*-----------------------------------------.---------------------------------.
| Filename: SimpleTreeViewItem.h           | Base class for tree view items  |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_SIMPLE_TREE_VIEW_ITEM_H_
# define LBCPP_USER_INTERFACE_SIMPLE_TREE_VIEW_ITEM_H_

# include "../common.h"
# include "UserInterfaceManager.h"

namespace lbcpp
{

class SimpleTreeViewItem : public juce::TreeViewItem
{
public:
  SimpleTreeViewItem(const String& uniqueName, juce::Image* iconToUse = 0, bool mightContainSubItems = false)
    : uniqueName(uniqueName), iconToUse(iconToUse), mightContainSubItemsFlag(mightContainSubItems), hasBeenOpened(false) {}

  SimpleTreeViewItem(const String& uniqueName, const String& iconToUse, bool mightContainSubItems = false)
    : uniqueName(uniqueName), iconToUse(userInterfaceManager().getImage(iconToUse, 18, 18)),
      mightContainSubItemsFlag(mightContainSubItems), hasBeenOpened(false) {}
     
  virtual bool mightContainSubItems()
    {return mightContainSubItemsFlag;}

  void setUniqueName(const String& name)
    {this->uniqueName = name;}

  virtual const String getUniqueName() const
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
  
  void setIcon(const String& name)
    {iconToUse = userInterfaceManager().getImage(name, 18, 18);}

  bool hasBeenOpenedOnce() const
    {return hasBeenOpened;}

protected:
  String uniqueName;
  juce::Image* iconToUse;
  bool mightContainSubItemsFlag;
  bool hasBeenOpened;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_SIMPLE_TREE_VIEW_ITEM_H_
