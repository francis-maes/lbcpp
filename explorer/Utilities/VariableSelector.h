/*-----------------------------------------.---------------------------------.
| Filename: VariableSelector.h             | Interface for variable selectors|
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_UTILITIES_VARIABLE_SELECTOR_H_
# define EXPLORER_UTILITIES_VARIABLE_SELECTOR_H_

# include "../Components/common.h"
# include "ComponentWithPreferedSize.h"

namespace lbcpp
{

class VariableSelector;
class VariableSelectorCallback
{
public:
  virtual ~VariableSelectorCallback() {}

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables) = 0;
};

class VariableSelector
{
public:
  void addCallback(VariableSelectorCallback& callback)
    {callbacks.push_back(&callback);}

  void sendSelectionChanged(const Variable& selectedVariable)
    {sendSelectionChanged(std::vector<Variable>(1, selectedVariable));}

  void sendSelectionChanged(const std::vector<Variable>& selectedVariables)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->selectionChangedCallback(this, selectedVariables);
  }

  virtual Variable createMultiSelectionVariable(const std::vector<Variable>& selection)
    {return lbcpp::createMultiSelectionVariable(selection);}

  virtual Component* createComponentForVariable(const Variable& variable, const String& name)
    {return lbcpp::createComponentForVariable(variable, name);}

protected:
  std::vector<VariableSelectorCallback* > callbacks;
};

class TabbedVariableSelectorComponent : public TabbedButtonBar, public VariableSelector, public juce::ChangeListener, public ComponentWithPreferedSize
{
public:
  TabbedVariableSelectorComponent(const Variable& variable)
    : TabbedButtonBar(TabsAtLeft), variable(variable)
    {addChangeListener(this);}

  virtual void changeListenerCallback(void* objectThatHasChanged)
    {sendSelectionChanged(Variable::pair(variable, getCurrentTabName()));}

  virtual int getDefaultWidth() const
    {return 27;}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {return 27;}

protected:
  Variable variable;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_VARIABLE_SELECTOR_H_
