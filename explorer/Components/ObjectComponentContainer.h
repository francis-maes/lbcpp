/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponentContainer.h     | A Object proxy component        |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 13:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_CONTAINER_H_
# define EXPLORER_COMPONENTS_CONTAINER_H_

# include "common.h"
# include "../Utilities/SplittedLayout.h"
# include "../Utilities/ObjectSelector.h"

namespace lbcpp
{

class ObjectComponentContainer : public Component
{
public:
  ObjectComponentContainer() : component(NULL)
    {}

  virtual ~ObjectComponentContainer()
    {deleteAllChildren();}

  void setObject(ObjectPtr object)
    {setComponent((this->object = object) ? createComponentForObject(object) : NULL);}
  
  ObjectPtr getObject() const
    {return object;}
  
  virtual void resized()
  {
    if (component)
      component->setBoundsRelative(0, 0, 1, 1);
  }
  
private:
  Component* component;
  ObjectPtr object;

  void setComponent(Component* newComponent)
  {
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
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_CONTAINER_H_

