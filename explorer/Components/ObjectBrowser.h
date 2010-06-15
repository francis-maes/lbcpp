/*-----------------------------------------.---------------------------------.
| Filename: ObjectBrowser.h                | The Object Browser              |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_BROWSER_H_
# define EXPLORER_COMPONENTS_OBJECT_BROWSER_H_

# include "ObjectProxyComponent.h"
# include "../Utilities/PropertyListDisplayComponent.h"
# include "../Utilities/ComponentWithPreferedSize.h"

namespace lbcpp
{

class ObjectSelectorAndContentComponent : public Component, public ObjectSelectorCallback, public ComponentWithPreferedSize
{
public:
  ObjectSelectorAndContentComponent(ObjectPtr object, Component* selector)
    : object(object), selector(selector), content(new ObjectProxyComponent())
  {
    ObjectSelector* s = dynamic_cast<ObjectSelector* >(selector);
    jassert(s);
    s->addCallback(*this);

    addAndMakeVisible(properties = new PropertyListDisplayComponent(40));
    properties->addProperty(T("Class"), object->getClassName());
    String name = object->getName();
    if (name.indexOf(T("unimplemented")) < 0)
      properties->addProperty(T("Name"), name);

    addAndMakeVisible(selector);
    addAndMakeVisible(resizeBar = new juce::StretchableLayoutResizerBar(&layout, 1, true));
    addAndMakeVisible(content);

    layout.setItemLayout(0, 10, -1, 200);
    double size = 4;
    layout.setItemLayout(1, size, size, size);
    layout.setItemLayout(2, 10, -1, -1);   
  }

  virtual ~ObjectSelectorAndContentComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    Component* comps[] = {selector, resizeBar, content};
    layout.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), false, true);
    
    enum {propertiesHeight = 25};
    properties->setBounds(selector->getX(), 0, selector->getWidth(), propertiesHeight);
    selector->setBounds(selector->getX(), propertiesHeight, selector->getWidth(), getHeight() - propertiesHeight);
  }

  virtual void objectSelectedCallback(ObjectPtr object)
    {if (object != this->object) content->setObject(object);}

  enum {selectorPreferedWidth = 200};

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    ComponentWithPreferedSize* content = dynamic_cast<ComponentWithPreferedSize* >(this->content);
    if (content)
    {
      int res = selector->getWidth() + resizeBar->getWidth();
      availableWidth -= res;
      return res + content->getPreferedWidth(availableWidth, availableHeight);
    }
    else
      return juce::jmax(selectorPreferedWidth, availableWidth);
  }

private:
  ObjectPtr object;
  PropertyListDisplayComponent* properties;
  Component* selector;
  Component* resizeBar;
  ObjectProxyComponent* content;
  juce::StretchableLayoutManager layout;
};

class ObjectBrowser : public ViewportComponent
{
public:
  ObjectBrowser(ObjectSelectorAndContentComponent* content)
    : ViewportComponent(content, false, true) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_BROWSER_H_

