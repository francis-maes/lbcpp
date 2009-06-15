/*-----------------------------------------.---------------------------------.
| Filename: SplittedLayout.h               | Horizontal or Vertical          |
| Author  : Francis Maes                   | Splitted Layout                 |
| Started : 10/12/2006 17:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_UTILITIES_SPLITTED_LAYOUT_H_
# define EXPLORER_UTILITIES_SPLITTED_LAYOUT_H_

# include "../Juce/juce_amalgamated.h"

class SplittedLayout : public Component
{
public:
  SplittedLayout(Component* first, Component* second, double default_ratio, int flags)
    : layoutflags(flags), first(first), second(second)
  {
    addAndMakeVisible(first);
    layout.setItemLayout(0, 10, -1, -default_ratio);
    if ((layoutflags & noResizeBar) != 0)
      resizebar = new Component();
    else
      resizebar = new StretchableLayoutResizerBar(&layout, 1, (layoutflags & vertical) == 0);
    addAndMakeVisible(resizebar);
    double s = (layoutflags & miniResizeBar) != 0 ? 2 : 8;
    layout.setItemLayout(1, s, s, s);
    addAndMakeVisible(second);
    layout.setItemLayout(2, 10, -1, default_ratio - 1);
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

  StretchableLayoutManager layout;

  juce_UseDebuggingNewOperator

protected:
  int        layoutflags;
  Component* first;
  Component* resizebar;
  Component* second;
};

#endif //!EXPLORER_UTILITIES_SPLITTED_LAYOUT_H_
