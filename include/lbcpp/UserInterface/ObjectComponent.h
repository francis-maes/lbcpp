/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponent.h              | Base classes for ui components  |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_OBJECT_COMPONENT_H_
# define LBCPP_USER_INTERFACE_OBJECT_COMPONENT_H_

# include "../common.h"
# include "../Core/Variable.h"

namespace lbcpp
{

using juce::Graphics;
using juce::Component;
using juce::Colours;

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
  ViewportComponent(Component* component = NULL, bool showVerticalScrollbarIfNeeded = true, bool showHorizontalScrollbarIfNeeded = true);

  virtual void resized();
};

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
  TabbedVariableSelectorComponent(const Variable& variable);

  virtual Variable getSubVariable(const Variable& variable, const String& tabName) const;
  virtual void changeListenerCallback(void* objectThatHasChanged);
  virtual int getDefaultWidth() const;
  virtual int getPreferedWidth(int availableWidth, int availableHeight) const;

protected:
  Variable variable;
};

class BooleanButtonsComponent : public Component, public juce::ButtonListener, public juce::ChangeBroadcaster
{
public:
  virtual ~BooleanButtonsComponent();

  void initialize();

  virtual void buttonClicked(juce::Button* button);
  virtual void paint(Graphics& g);
  virtual void resized();

  enum {buttonWidth = 200, buttonHeight = 20};

protected:
  struct ConfigurationButton : public juce::ToggleButton
  {
    ConfigurationButton(const String& name, bool& value, const juce::Colour& colour = juce::Colours::black)
      : ToggleButton(name), value(value)
    {
      setColour(textColourId, colour);
      setToggleState(value, false);
    }
      
    bool& value;
  };

  std::vector< std::vector<ConfigurationButton* > > buttons;

  void addToggleButton(std::vector<ConfigurationButton* >& buttonsColumn, const String& name, bool& state, size_t columnsHeight, const juce::Colour& colour = juce::Colours::black);
  void flushButtons(std::vector<ConfigurationButton* >& buttonsColumn);
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_OBJECT_COMPONENT_H_
