/*-----------------------------------------.---------------------------------.
| Filename: ProcessComponent.cpp           | Process UI Component            |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 17:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProcessManagerComponent.h"
#include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
using namespace lbcpp;

class ProcessConsoleComponent : public Component, public ComponentWithPreferedSize
{
public:
  ProcessConsoleComponent(ProcessPtr process, ProcessConsoleSettingsPtr settings)
    : process(process), settings(settings)
    {jassert(settings); setOpaque(true);}

  void updateContent()
  {
    int dh = getPreferedHeight(getWidth(), getHeight());
    if (getHeight() < dh)
      setSize(getWidth(), dh);
    repaint();
  }

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
  {
    int numLines = 0;
    for (size_t i = 0; i < process->getProcessOutput().size(); ++i)
    {
      bool display = true;
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
      bool display = true;
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
ProcessComponent::ProcessComponent(ProcessPtr process, ProcessConsoleSettingsPtr settings) : process(process)
{
  jassert(settings);
  addAndMakeVisible(properties = new PropertyListDisplayComponent());
  properties->addProperty(T("Executable"), process->getExecutableFile().getFullPathName());
  properties->addProperty(T("Arguments"), process->getArguments());
  properties->addProperty(T("Working Directory"), process->getWorkingDirectory().getFullPathName());

  console = new ProcessConsoleComponent(process, settings);
  addAndMakeVisible(viewport = new ViewportComponent(console, true, false));
  if (settings)
    addAndMakeVisible(consoleTools = settings->createComponent());
  else
    consoleTools = NULL;
}

ProcessComponent::~ProcessComponent()
  {deleteAllChildren();}

void ProcessComponent::resized()
{
  int propertiesHeight = 50;
  int consoleToolsHeight = 20;

  properties->setBounds(0, 0, getWidth(), propertiesHeight);
  viewport->setBounds(0, propertiesHeight, getWidth(), getHeight() - propertiesHeight - consoleToolsHeight);
  int w = viewport->getMaximumVisibleWidth();
  int h = viewport->getMaximumVisibleHeight();
  if (console)
    console->setSize(viewport->getWidth(), juce::jmax(console->getPreferedHeight(w, h), viewport->getHeight()));
  if (consoleTools)
    consoleTools->setBounds(0, getHeight() - consoleToolsHeight, getWidth(), consoleToolsHeight);
}

void ProcessComponent::updateContent()
  {console->updateContent();}

juce::Component* Process::createComponent() const
  {return new ProcessComponent(ProcessPtr(const_cast<Process* >(this)));}

juce::Component* Process::createComponent(ProcessConsoleSettingsPtr settings) const
  {return new ProcessComponent(ProcessPtr(const_cast<Process* >(this)), settings);}
