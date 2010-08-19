/*-----------------------------------------.---------------------------------.
| Filename: VariableProxyComponent.h       | A component whose Variable can  |
| Author  : Francis Maes                   | be dynamically changed          |
| Started : 15/06/2009 13:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_PROXY_H_
# define EXPLORER_COMPONENTS_VARIABLE_PROXY_H_

# include "common.h"
# include "../Utilities/ComponentWithPreferedSize.h"

namespace lbcpp
{

class VariableProxyComponent : public Component, public ComponentWithPreferedSize
{
public:
  VariableProxyComponent() : component(NULL)
    {}

  virtual ~VariableProxyComponent()
    {deleteAllChildren();}

  void setVariable(const Variable& variable)
    {setVariable(variable, createComponentForVariable(variable));}
  
  void setVariable(const Variable& variable, Component* newComponent)
  {
    this->variable = variable;
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
    
  Variable getVariable() const
    {return variable;}
  
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
  Variable variable;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_PROXY_H_
