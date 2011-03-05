/*-----------------------------------------.---------------------------------.
| Filename: ProcessConsoleSettingsCom...cpp| Process UI Component            |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 17:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ProcessManagerComponent.h"
#include "../ExplorerConfiguration.h"
using namespace lbcpp;

class ProcessConsoleFilterComponent : public Component, public juce::ButtonListener
{
public:
  ProcessConsoleFilterComponent(ExecutionContext& context, ProcessConsoleFilterPtr filter)
    : context(context), filter(filter)
  {
    addAndMakeVisible(displayButton = new juce::ToggleButton(String::empty));
    displayButton->setToggleState(filter->getDisplayFlag(), false);
    displayButton->addButtonListener(this);

    addAndMakeVisible(configureButton = new juce::TextButton(filter->getPattern()));
    configureButton->addButtonListener(this);
  }

  virtual ~ProcessConsoleFilterComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    displayButton->setBoundsRelative(0.05f, 0.1f, 0.25f, 0.8f);
    configureButton->setBoundsRelative(0.35f, 0.1f, 0.6f, 0.8f);
  }

  virtual void paint(Graphics& g)
    {g.fillAll(filter->getColour());}

  virtual void buttonClicked(juce::Button* button)
  {
    if (button == configureButton)
    {
      AlertWindow alertWindow(T("Configure pattern"), T("Choose a new pattern"), AlertWindow::QuestionIcon);
      alertWindow.addTextEditor(T("pattern"), filter->getPattern(), T("Pattern:"));
      alertWindow.addButton(T("OK"), 1, juce::KeyPress::returnKey);
      alertWindow.addButton(T("Cancel"), 0, juce::KeyPress::escapeKey);
      if (alertWindow.runModalLoop())
      {
        filter->setPattern(alertWindow.getTextEditorContents(T("pattern")));
        configureButton->setButtonText(filter->getPattern());
        ExplorerConfiguration::save(context);
      }
    }
    else if (button == displayButton)
    {
      filter->setDisplayFlag(displayButton->getToggleState());
      ExplorerConfiguration::save(context);
    }
  }

  juce_UseDebuggingNewOperator

private:
  ExecutionContext& context;
  ProcessConsoleFilterPtr filter;
  juce::Button* configureButton;
  juce::Button* displayButton;
};

juce::Component* ProcessConsoleFilter::createComponent() const
  {return new ProcessConsoleFilterComponent(defaultExecutionContext(), ProcessConsoleFilterPtr(const_cast<ProcessConsoleFilter* >(this)));}

class ProcessConsoleSettingsComponent : public Component
{
public:
  ProcessConsoleSettingsComponent(ProcessConsoleSettingsPtr settings)
    : settings(settings)
  {
    filters.resize(settings->getNumFilters());
    for (size_t i = 0; i < filters.size(); ++i)
      addAndMakeVisible(filters[i] = settings->getFilter(i)->createComponent());
  }
  virtual ~ProcessConsoleSettingsComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    for (int i = 0; i < (int)filters.size(); ++i)
    {
      int x1 = (i * getWidth() / (int)filters.size());
      int x2 = ((i + 1) * getWidth() / (int)filters.size());
      filters[i]->setBounds(x1, 0, x2 - x1, getHeight());
    }
  }

  juce_UseDebuggingNewOperator

private:
  ProcessConsoleSettingsPtr settings;
  std::vector<Component* > filters;
};

juce::Component* ProcessConsoleSettings::createComponent() const
  {return new ProcessConsoleSettingsComponent(ProcessConsoleSettingsPtr(const_cast<ProcessConsoleSettings* >(this)));}
