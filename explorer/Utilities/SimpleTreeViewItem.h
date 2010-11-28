/*-----------------------------------------.---------------------------------.
| Filename: SimpleTreeViewItem.h           | Base class for tree view items  |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_UTILITIES_SIMPLE_TREE_VIEW_ITEM_H_
# define EXPLORER_UTILITIES_SIMPLE_TREE_VIEW_ITEM_H_

# include "../Components/common.h"

namespace lbcpp
{

class SimpleTreeViewItem : public juce::TreeViewItem
{
public:
  SimpleTreeViewItem(const String& uniqueName, juce::Image* iconToUse = 0, bool mightContainSubItems = false)
    : uniqueName(uniqueName), iconToUse(iconToUse), mightContainSubItemsFlag(mightContainSubItems), hasBeenOpened(false) {}
     
  virtual bool mightContainSubItems()
    {return mightContainSubItemsFlag;}

  virtual const String getUniqueName() const
    {return uniqueName;}
    
  enum {defaultIconSize = 18};

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
    g.drawText(getUniqueName(), x1, 0, width - x1, height, Justification::centredLeft, true);
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
  
protected:
  String uniqueName;
  juce::Image* iconToUse;
  bool mightContainSubItemsFlag;
  bool hasBeenOpened;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_SIMPLE_TREE_VIEW_ITEM_H_
