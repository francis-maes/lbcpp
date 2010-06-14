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
  ObjectComponentContainer() : component(NULL) {}
  virtual ~ObjectComponentContainer()
    {deleteAllChildren();}
  
  void setObject(ObjectPtr object)
  {
    deleteAllChildren();
    if (object)
    {
      component = createComponentForObject(object);
      addAndMakeVisible(component);
      component->setBoundsRelative(0, 0, 1, 1);
    }
    else
      component = NULL;
    this->object = object;
  }
  
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
};

class ObjectSelectorAndContentComponent : public SplittedLayout, public ObjectSelectorCallback
{
public:
  ObjectSelectorAndContentComponent(Component* selector)
    : SplittedLayout(selector, new ObjectComponentContainer(), 0.33, typicalHorizontal)
  {
    ObjectSelector* s = dynamic_cast<ObjectSelector* >(selector);
    jassert(s);
    s->addCallback(*this);
  }

  virtual void objectSelectedCallback(ObjectPtr selected)
    {((ObjectComponentContainer* )second)->setObject(selected);}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_CONTAINER_H_

