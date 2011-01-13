/*-----------------------------------------.---------------------------------.
| Filename: SplittedLayout.h               | Horizontal or Vertical          |
| Author  : Francis Maes                   | Splitted Layout                 |
| Started : 10/12/2006 17:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_UTILITIES_SPLITTED_LAYOUT_H_
# define EXPLORER_UTILITIES_SPLITTED_LAYOUT_H_

# include "../Components/common.h"
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>

namespace lbcpp
{

class SplittedLayout : public Component, public ComponentWithPreferedSize
{
public:
  SplittedLayout(Component* first, Component* second, double defaultRatio, int flags)
    : layoutflags(flags), first(first), second(second), defaultRatio(defaultRatio)
  {
    addAndMakeVisible(first);
    layout.setItemLayout(0, 10, -1, -defaultRatio);
    if ((layoutflags & noResizeBar) != 0)
      resizebar = new Component();
    else
      resizebar = new juce::StretchableLayoutResizerBar(&layout, 1, (layoutflags & vertical) == 0);
    addAndMakeVisible(resizebar);
    double s = (layoutflags & miniResizeBar) != 0 ? 2 : 8;
    layout.setItemLayout(1, s, s, s);
    addAndMakeVisible(second);
    layout.setItemLayout(2, 10, -1, defaultRatio - 1);
  }

  virtual ~SplittedLayout()
  {
    if ((layoutflags & deleteChildrens) != 0)
      deleteAllChildren();
    else
    {
      removeChildComponent(resizebar);
      delete resizebar;
    }
  }

  enum
  {
    horizontal = 0x00,
    vertical = 0x01,
    noResizeBar = 0x02,
    miniResizeBar = 0x04,
    deleteChildrens = 0x08,
    borderPaintOverChildren = 0x10,

    typicalVertical = vertical | deleteChildrens | miniResizeBar,
    typicalHorizontal = horizontal | deleteChildrens | miniResizeBar,
    defaultForToolbars = vertical | miniResizeBar | deleteChildrens,
  };

  virtual void resized()
  {
    Component* comps[] = {first, resizebar, second};
    layout.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), (layoutflags & vertical) != 0, true);
  }

  virtual void paintOverChildren(Graphics& g)
  {
    if ((layoutflags & borderPaintOverChildren) != 0)
    {
      g.setColour(Colours::black);
      if ((layoutflags & vertical) != 0)
      {
        g.drawLine(0.f, (float)first->getBottom() - 1, (float)getWidth(), (float)first->getBottom() - 1);
        g.drawLine(0.f, (float)second->getY(), (float)getWidth(), (float)second->getY());
      }
      else
      {
        g.drawLine((float)first->getRight() - 1, 0.f, (float)first->getRight() - 1, (float)getHeight());
        g.drawLine((float)second->getX(), 0.f, (float)second->getX(), (float)getHeight());
      }
    }
  }
  
  void changeFirstComponent(Component* newComponent)
  {
    if (first)
      removeChildComponent(first), delete first;
    addAndMakeVisible(first = newComponent);
    resized();
  }

  void changeSecondComponent(Component* newComponent)
  {
    if (second)
      removeChildComponent(second), delete second;
    addAndMakeVisible(second = newComponent);
    resized();
  }
  
  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {return isHorizontal() ? computePreferedSize(availableWidth, availableHeight) : availableWidth;}

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
    {return isVertical() ? computePreferedSize(availableHeight, availableWidth) : availableHeight;}

  int computePreferedSize(int availableSize1, int availableSize2) const
  {
    int prefered1 = getSubPreferedSize(first, (int)(availableSize1 * defaultRatio), availableSize2);
    int prefered2 = getSubPreferedSize(second, (int)(availableSize1 * (1.0 - defaultRatio)), availableSize2);
    return prefered1 + 8 + prefered2;
  }

  int getSubPreferedSize(Component* c, int availableSize1, int availableSize2) const
  {
    ComponentWithPreferedSize* cs = dynamic_cast<ComponentWithPreferedSize* >(c);
    return cs 
      ? (isHorizontal() ? cs->getPreferedWidth(availableSize1, availableSize2) : cs->getPreferedHeight(availableSize2, availableSize1))
      : availableSize1;
  }

  bool isHorizontal() const
    {return (layoutflags & horizontal) == horizontal;}

  bool isVertical() const
    {return (layoutflags & vertical) == vertical;}

  juce::StretchableLayoutManager layout;

  juce_UseDebuggingNewOperator

protected:
  int        layoutflags;
  Component* first;
  Component* resizebar;
  Component* second;
  double defaultRatio;
};

}; /* namespace lbcpp */

#endif //!EXPLORER_UTILITIES_SPLITTED_LAYOUT_H_
