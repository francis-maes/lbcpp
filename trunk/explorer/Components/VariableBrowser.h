/*-----------------------------------------.---------------------------------.
| Filename: VariableBrowser.h              | The Variable Browser            |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_
# define EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_

# include "VariableProxyComponent.h"
# include "../Utilities/PropertyListDisplayComponent.h"
# include "../Utilities/ComponentWithPreferedSize.h"
# include "../Utilities/VariableSelector.h"

class VariableBrowserContent;

namespace lbcpp
{

#if 0
class VariableSelectorAndContentComponent : public Component, public VariableSelectorCallback, public ComponentWithPreferedSize
{
public:
  VariableSelectorAndContentComponent(const Variable& variable, Component* selector)
    : variable(variable), selector(selector), content(new VariableProxyComponent())
  {
    VariableSelector* s = dynamic_cast<VariableSelector* >(selector);
    jassert(s);
    s->addCallback(*this);

    addAndMakeVisible(properties = new PropertyListDisplayComponent(40));
    properties->addProperty(T("Type"), variable.getTypeName());
    String name = variable.isObject() ? variable.getObject()->getName() : String::empty;
    if (name.isNotEmpty() && name.indexOf(T("unimplemented")) < 0)
      properties->addProperty(T("Name"), name);

    addAndMakeVisible(selector);
    addAndMakeVisible(resizeBar = new juce::StretchableLayoutResizerBar(&layout, 1, true));
    addAndMakeVisible(content);

    double preferedWidth = 200.0;
    ComponentWithPreferedSize* componentWithPreferedSize = dynamic_cast<ComponentWithPreferedSize* >(selector);
    if (componentWithPreferedSize)
    {
      int w = componentWithPreferedSize->getPreferedWidth(0, 0);
      if (w)
        preferedWidth = (double)w;
    }
    layout.setItemLayout(0, 10, -1, preferedWidth);
    double size = 4;
    layout.setItemLayout(1, size, size, size);
    layout.setItemLayout(2, 10, -1, -1.0);
  }

  virtual ~VariableSelectorAndContentComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    Component* comps[] = {selector, resizeBar, content};
    layout.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), false, true);
    
    enum {propertiesHeight = 25};
    properties->setBounds(selector->getX(), 0, selector->getWidth(), propertiesHeight);
    selector->setBounds(selector->getX(), propertiesHeight, selector->getWidth(), getHeight() - propertiesHeight);
  }

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables)
  {
    std::vector<Variable> variables;
    variables.reserve(selectedVariables.size());
    for (size_t i = 0; i < selectedVariables.size(); ++i)
      if (selectedVariables[i] != variable) // it is not possible to select the root effect
        variables.push_back(selectedVariables[i]);
    Variable multiSelection = createMultiSelectionVariable(variables);
    content->setVariable(multiSelection);
  }

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
  Variable variable;
  PropertyListDisplayComponent* properties;
  Component* selector;
  Component* resizeBar;
  VariableProxyComponent* content;
  juce::StretchableLayoutManager layout;
};
#endif // 0

class VariableBrowser : public ViewportComponent
{
public:
  VariableBrowser(const Variable& variable, Component* selector);

  virtual void resized();

private:
  VariableBrowserContent* getContent() const;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_

