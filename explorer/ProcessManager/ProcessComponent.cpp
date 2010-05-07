/*-----------------------------------------.---------------------------------.
| Filename: ProcessComponent.cpp           | Process UI Component            |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 17:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProcessManagerComponent.h"
using namespace lbcpp;

class ProcessPropertiesComponent : public Component
{
public:
  ProcessPropertiesComponent(ProcessPtr process)
  {
    addAndMakeVisible(executableProperty = new PropertyComponent(T("Executable"), process->getExecutableFile().getFullPathName()));
    addAndMakeVisible(argumentsProperty = new PropertyComponent(T("Arguments"), process->getArguments()));
    addAndMakeVisible(workingDirectoryProperty = new PropertyComponent(T("Working Directory"), process->getWorkingDirectory().getFullPathName()));
    setOpaque(true);
  }

  virtual ~ProcessPropertiesComponent()
    {deleteAllChildren();}
  
  virtual void resized()
  {
    int w = getWidth();
    int h = getHeight();
    executableProperty->setBounds(0, 0, w, h / 3);
    argumentsProperty->setBounds(0, h / 3, w, h / 3);
    workingDirectoryProperty->setBounds(0, 2 * h / 3, w, h / 3);
  }

  virtual void paint(Graphics& g)
    {g.fillAll(Colours::antiquewhite);}

  class PropertyComponent : public Component
  {
  public:
    PropertyComponent(const String& name, const String& value)
      : name(name), value(value) {}

    virtual void paint(Graphics& g)
    {
      int nameWidth = 120;

      Font nameFont(12, Font::bold);
      g.setFont(nameFont);
      g.drawText(name, 0, 0, nameWidth, getHeight(), Justification::centredLeft, true);
      
      Font valueFont(12, Font::plain);
      g.setFont(valueFont);
      g.drawText(value, nameWidth, 0, getWidth() - nameWidth, getHeight(), Justification::centredLeft, true);
    }

    juce_UseDebuggingNewOperator

  private:
    String name, value;
  };

  juce_UseDebuggingNewOperator

private:
  PropertyComponent* executableProperty;
  PropertyComponent* argumentsProperty;
  PropertyComponent* workingDirectoryProperty;
};

class ProcessConsoleComponent : public Component
{
public:
  ProcessConsoleComponent(ProcessPtr process, ProcessConsoleSettingsPtr settings)
    : process(process), settings(settings)
    {setOpaque(true);}

  void updateContent()
  {
    int dh = getDesiredHeight();
    if (getHeight() < dh)
      setSize(getWidth(), dh);
    repaint();
  }

  int getDesiredHeight() const
  {
    int numLines = 0;
    for (size_t i = 0; i < process->getProcessOutput().size(); ++i)
    {
      bool display;
      settings->getColourForLine(process->getProcessOutput()[i], display);
      if (display)
        ++numLines;
    }
    return 12 * numLines;
  }

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::black);
    int y = 0;
    for (size_t i = 0; i < process->getProcessOutput().size(); ++i)
    {
      String line = process->getProcessOutput()[i];
      bool display;
      g.setColour(settings->getColourForLine(line, display));
      if (display)
      {
        g.drawText(line, 0, y, getWidth(), 12, Justification::centredLeft, false);
        y += 12;
      }
    }
  }

  juce_UseDebuggingNewOperator

private:
  ProcessPtr process;
  ProcessConsoleSettingsPtr settings;
};


/*
** ProcessComponent
*/
ProcessComponent::ProcessComponent(ProcessPtr process) : process(process)
{
  ProcessConsoleSettingsPtr settings = new ProcessConsoleSettings(); // todo: serialize
  settings->addFilter(new ProcessConsoleFilter(T("ITERATION"), Colours::red));
  settings->addFilter(new ProcessConsoleFilter(T("L2 norm ="), Colours::orange));
  settings->addFilter(new ProcessConsoleFilter(T("SS3"), Colours::yellow));
  settings->addFilter(new ProcessConsoleFilter(T("DR"), Colours::green));
  settings->addFilter(new ProcessConsoleFilter(String::empty, Colours::cyan));
  settings->addFilter(new ProcessConsoleFilter(String::empty, Colours::white));

  addAndMakeVisible(properties = new ProcessPropertiesComponent(process));
  addAndMakeVisible(viewport = new Viewport());
  addAndMakeVisible(consoleTools = settings->createComponent());
  console = new ProcessConsoleComponent(process, settings);
  viewport->setViewedComponent(console);
  viewport->setScrollBarsShown(true, false);
}

ProcessComponent::~ProcessComponent()
  {deleteAllChildren();}

void ProcessComponent::resized()
{
  int propertiesHeight = 50;
  int consoleToolsHeight = 20;

  properties->setBounds(0, 0, getWidth(), propertiesHeight);
  viewport->setBounds(0, propertiesHeight, getWidth(), getHeight() - propertiesHeight - consoleToolsHeight);
  if (console)
    console->setSize(viewport->getWidth(), juce::jmax(console->getDesiredHeight(), viewport->getHeight()));
  consoleTools->setBounds(0, getHeight() - consoleToolsHeight, getWidth(), consoleToolsHeight);
}

void ProcessComponent::updateContent()
  {console->updateContent();}

juce::Component* Process::createComponent() const
  {return new ProcessComponent(ProcessPtr(const_cast<Process* >(this)));}

