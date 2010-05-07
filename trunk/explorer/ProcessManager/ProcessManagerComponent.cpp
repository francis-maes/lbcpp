/*-----------------------------------------.---------------------------------.
| Filename: ProcessManagerComponent.cpp    | Process Manager UI Component    |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProcessManagerComponent.h"
#include "NewProcessDialogWindow.h"
#include "../Components/ObjectContainerNameListComponent.h"
using namespace lbcpp;

class ProcessConsoleComponent : public Component
{
public:
  ProcessConsoleComponent(ProcessPtr process) : process(process)
    {setOpaque(true);}

  void updateContent()
  {
    int dh = getDesiredHeight();
    if (getHeight() < dh)
      setSize(getWidth(), dh);
    repaint();
  }

  int getDesiredHeight() const
    {return 12 * process->getProcessOutput().size();}

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colours::black);
    g.setColour(Colours::white);
    for (size_t i = 0; i < process->getProcessOutput().size(); ++i)
      g.drawText(process->getProcessOutput()[i], 0, i * 12, getWidth(), 12, Justification::centredLeft, false);
  }

  juce_UseDebuggingNewOperator

private:
  ProcessPtr process;
};

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

  private:
    String name, value;
  };

private:
  PropertyComponent* executableProperty;
  PropertyComponent* argumentsProperty;
  PropertyComponent* workingDirectoryProperty;
};

class ProcessComponent : public Component
{
public:
  ProcessComponent(ProcessPtr process) : process(process)
  {
    addAndMakeVisible(properties = new ProcessPropertiesComponent(process));
    addAndMakeVisible(viewport = new Viewport());
    console = new ProcessConsoleComponent(process);
    viewport->setViewedComponent(console);
    viewport->setScrollBarsShown(true, false);
  }

  virtual ~ProcessComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    int propertiesHeight = 50;
    properties->setBounds(0, 0, getWidth(), propertiesHeight);
    viewport->setBounds(0, propertiesHeight, getWidth(), getHeight() - propertiesHeight);
    if (console)
      console->setSize(viewport->getWidth(), juce::jmax(console->getDesiredHeight(), viewport->getHeight()));
  }

  void updateContent()
    {console->updateContent();}

protected:
  ProcessPtr process;

  ProcessPropertiesComponent* properties;

  Viewport* viewport;
  ProcessConsoleComponent* console;
};

juce::Component* Process::createComponent() const
  {return new ProcessComponent(ProcessPtr(const_cast<Process* >(this)));}

class ProcessManagerListTabs : public TabbedComponent
{
public:
  ProcessManagerListTabs(ProcessManagerComponent* owner, ProcessManagerPtr processManager)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom), owner(owner)
  {
    addProcessList(T("Running"), processManager->getRunningProcesses());
    addProcessList(T("Waiting"), processManager->getWaitingProcesses());
    addProcessList(T("Finished"), processManager->getFinishedProcesses());
    addProcessList(T("Killed"), processManager->getKilledProcesses());
  }

  void updateContent()
  {
    for (int i = 0; i < getNumTabs(); ++i)
    {
      juce::ListBox* c = dynamic_cast<juce::ListBox* >(getTabContentComponent(i));
      jassert(c);
      c->updateContent();
    }
  }

  void selectProcess(ProcessPtr process)
  {
    for (int i = 0; i < getNumTabs(); ++i)
    {
      ObjectContainerNameListComponent* c = dynamic_cast<ObjectContainerNameListComponent* >(getTabContentComponent(i));
      jassert(c);
      int index = c->getContainer()->findObject(process);
      if (index >= 0)
      {
        c->selectRow(index);
        owner->processSelectedCallback(process);
        return;
      }
    }
  }

  juce_UseDebuggingNewOperator

private:
  ProcessManagerComponent* owner;

  class ProcessListComponent : public ObjectContainerNameListComponent
  {
  public:
    ProcessListComponent(ProcessManagerComponent* owner, ProcessListPtr processes)
      : ObjectContainerNameListComponent(processes), owner(owner) {}

    virtual void objectSelectedCallback(size_t index, ObjectPtr object)
    {
      ProcessPtr process = object.dynamicCast<Process>();
      jassert(process);
      owner->processSelectedCallback(process);
    }

  private:
    ProcessManagerComponent* owner;
  };

  void addProcessList(const String& name, ProcessListPtr processes)
    {addTab(name, Colours::antiquewhite, new ProcessListComponent(owner, processes), true);}
};

/*
** ProcessManagerComponent
*/
ProcessManagerComponent::ProcessManagerComponent(ProcessManagerPtr processManager)
  : SplittedLayout(new ProcessManagerListTabs(this, processManager), NULL, 0.33, SplittedLayout::typicalHorizontal), processManager(processManager)
{
  startTimer(100);
}

void ProcessManagerComponent::timerCallback()
{
  processManager->updateProcesses();
  updateProcessLists();
}

void ProcessManagerComponent::updateProcessLists()
{
  ((ProcessManagerListTabs* )first)->updateContent();
  
  ProcessComponent* currentProcessComponent = (ProcessComponent* )second;
  if (currentProcessComponent)
    currentProcessComponent->updateContent();

  resized();
}

const StringArray ProcessManagerComponent::getMenuBarNames()
{
  StringArray res;
  res.add(T("Process"));
  return res;
}

const PopupMenu ProcessManagerComponent::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
  jassert(topLevelMenuIndex == 0);
  PopupMenu menu;
  menu.addItem(1, T("New Process"));
  menu.addItem(2, T("Clear Finished Process Lists"));
  menu.addItem(3, T("Kill all Processes"));
  return menu;
}

void ProcessManagerComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
  jassert(topLevelMenuIndex == 0);
  
  switch (menuItemID)
  {
  case 1:
    {
      File executable;
      String arguments;
      File workingDirectory;
      if (NewProcessDialogWindow::run(executable, arguments, workingDirectory))
      {
        ProcessPtr process = processManager->addNewProcess(executable, arguments, workingDirectory);
        if (process)
        {
          RecentProcessesPtr recents = RecentProcesses::getInstance();
          recents->addRecent(executable, arguments, workingDirectory);
          ((ProcessManagerListTabs* )first)->selectProcess(process);
        }
      }
    }
    break;

  case 2:
    processManager->clearFinishedProcessLists();
    updateProcessLists();
    break;

  case 3:
    processManager->killAllRunningProcesses();
    updateProcessLists();
    break;
  };
}

void ProcessManagerComponent::processSelectedCallback(ProcessPtr process)
{
  changeSecondComponent(process->createComponent());
}


juce::Component* ProcessManager::createComponent() const
  {return new ProcessManagerComponent(ProcessManagerPtr(const_cast<ProcessManager* >(this)));}
