/*-----------------------------------------.---------------------------------.
| Filename: VariableSelector.h             | Interface for variable selectors|
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_VARIABLE_SELECTOR_H_
# define LBCPP_USER_INTERFACE_VARIABLE_SELECTOR_H_

# include "ComponentWithPreferedSize.h"
# include "../Core/Variable.h"

namespace lbcpp
{

class VariableSelector;
class VariableSelectorCallback
{
public:
  virtual ~VariableSelectorCallback() {}

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables, const String& selectionName) = 0;
};

class VariableSelector
{
public:
  virtual ~VariableSelector() {}
  
  void addCallback(VariableSelectorCallback& callback)
    {callbacks.push_back(&callback);}

  void sendSelectionChanged(const Variable& selectedVariable, const String& selectionName)
    {sendSelectionChanged(std::vector<Variable>(1, selectedVariable), selectionName);}

  void sendSelectionChanged(const std::vector<Variable>& selectedVariables, const String& selectionName)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->selectionChangedCallback(this, selectedVariables, selectionName);
  }

  virtual juce::Component* createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& name)
    {return NULL;}

protected:
  std::vector<VariableSelectorCallback* > callbacks;
};

class TabbedVariableSelectorComponent : public juce::TabbedButtonBar, public VariableSelector, public juce::ChangeListener, public ComponentWithPreferedSize
{
public:
  TabbedVariableSelectorComponent(const Variable& variable)
    : TabbedButtonBar(TabsAtLeft), variable(variable)
    {addChangeListener(this);}

  virtual void changeListenerCallback(void* objectThatHasChanged)
    {sendSelectionChanged(variable, getCurrentTabName());}

  virtual int getDefaultWidth() const
    {return 27;}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {return 27;}

protected:
  Variable variable;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_VARIABLE_SELECTOR_H_
