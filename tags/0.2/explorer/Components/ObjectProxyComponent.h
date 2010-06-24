/*-----------------------------------------.---------------------------------.
| Filename: ObjectProxyComponent.h         | A Object proxy component        |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 13:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_PROXY_H_
# define EXPLORER_COMPONENTS_OBJECT_PROXY_H_

# include "common.h"
# include "../Utilities/ComponentWithPreferedSize.h"

namespace lbcpp
{

class ObjectProxyComponent : public Component, public ComponentWithPreferedSize
{
public:
  ObjectProxyComponent() : component(NULL)
    {}

  virtual ~ObjectProxyComponent()
    {deleteAllChildren();}

  void setObject(ObjectPtr object)
    {setObject(object, object ? createComponentForObject(object) : NULL);}
  
  void setObject(ObjectPtr object, Component* newComponent)
  {
    this->object = object;
    if (component)
    {
      removeChildComponent(component);
      delete component;
    }
    component = newComponent;
    if (newComponent)
    {
      addAndMakeVisible(newComponent);
      resized();
    }
  }
    
  ObjectPtr getObject() const
    {return object;}
  
  virtual void resized()
  {
    if (component)
      component->setBoundsRelative(0, 0, 1, 1);
  }
  
  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    ComponentWithPreferedSize* c = dynamic_cast<ComponentWithPreferedSize* >(component);
    return c ? c->getPreferedWidth(availableWidth, availableHeight) : availableWidth;
  }

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    ComponentWithPreferedSize* c = dynamic_cast<ComponentWithPreferedSize* >(component);
    return c ? c->getPreferedHeight(availableWidth, availableHeight) : availableHeight;
  }

private:
  Component* component;
  ObjectPtr object;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_PROXY_H_

