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
  RecentProcessesPtr recents = RecentProcesses::getInstance();
  ProcessConsoleSettingsPtr settings = recents->getExecutableConsoleSettings(process->getExecutableFile());
  changeSecondComponent(process->createComponent(settings));
}


juce::Component* ProcessManager::createComponent() const
  {return new ProcessManagerComponent(ProcessManagerPtr(const_cast<ProcessManager* >(this)));}
