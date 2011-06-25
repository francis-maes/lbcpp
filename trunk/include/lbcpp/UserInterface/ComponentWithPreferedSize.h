/*-----------------------------------------.---------------------------------.
| Filename: ComponentWithPreferedSize.h    | Component with prefered size    |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2010 14:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_COMPONENT_WITH_PREFERED_SIZE
# define LBCPP_USER_INTERFACE_COMPONENT_WITH_PREFERED_SIZE

# include "../common.h"

namespace lbcpp
{

class ComponentWithPreferedSize
{
public:
  virtual ~ComponentWithPreferedSize() {}

  virtual int getDefaultWidth() const
    {return getPreferedWidth(0, 0);}

  virtual int getDefaultHeight() const
    {return getPreferedHeight(0, 0);}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {return availableWidth;}

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
    {return availableHeight;}
};

class ViewportComponent : public juce::Viewport
{
public:
  ViewportComponent(Component* component = NULL, bool showVerticalScrollbarIfNeeded = true, bool showHorizontalScrollbarIfNeeded = true)
  {
    setScrollBarsShown(showVerticalScrollbarIfNeeded, showHorizontalScrollbarIfNeeded);
    if (component)
      setViewedComponent(component);
  }

  virtual void resized()
  {
    Component* content = getViewedComponent();
    if (content)
    {
      ComponentWithPreferedSize* c = dynamic_cast<ComponentWithPreferedSize* >(content);
      int w = getMaximumVisibleWidth();
      int h = getMaximumVisibleHeight();
      if (c)
        content->setSize(c->getPreferedWidth(w, h), c->getPreferedHeight(w, h));
      else
        content->setSize(w, h);
    }
    juce::Viewport::resized();
  }  
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENT_WITH_PREFERED_SIZE