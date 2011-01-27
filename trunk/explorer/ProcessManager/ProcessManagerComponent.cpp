/*-----------------------------------------.---------------------------------.
| Filename: ProcessManagerComponent.cpp    | Process Manager UI Component    |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProcessManagerComponent.h"
#include "NewProcessDialogWindow.h"
#include "../Components/ContainerSelectorComponent.h"
using namespace lbcpp;

class ProcessManagerListTabs : public TabbedComponent
{
public:
  ProcessManagerListTabs(ProcessManagerPtr processManager, VariableSelectorCallback& selectorCallback)
    : TabbedComponent(TabbedButtonBar::TabsAtBottom), selectorCallback(selectorCallback)
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
      ContainerSelectorComponent* c = dynamic_cast<ContainerSelectorComponent* >(getTabContentComponent(i));
      jassert(c);
      int index = c->getContainer()->findElement(process);
      if (index >= 0)
      {
        c->selectRow(index);
        return;
      }
    }
  }

  juce_UseDebuggingNewOperator

private:
  VariableSelectorCallback& selectorCallback;

  void addProcessList(const String& name, ProcessListPtr processes)
  {
    ContainerSelectorComponent* list = new ContainerSelectorComponent(processes);
    list->addCallback(selectorCallback);
    addTab(name, Colours::antiquewhite, list, true);
  }
};

/*
** ProcessManagerComponent
*/
ProcessManagerComponent::ProcessManagerComponent(ProcessManagerPtr processManager)
  : SplittedLayout(new ProcessManagerListTabs(processManager, *this), NULL, 0.33, SplittedLayout::typicalHorizontal), processManager(processManager)
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
  menu.addSeparator();
  menu.addItem(4, T("Clear Recent Processes Information"));
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

  case 4:
    RecentProcesses::getInstance()->clear();
    break;
  };
}

void ProcessManagerComponent::selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables, const String& selectionName)
{
  if (selectedVariables.size() == 1)
  {
    ProcessPtr process = selectedVariables[0].getObjectAndCast<Process>();
    jassert(process);
    RecentProcessesPtr recents = RecentProcesses::getInstance();
    ProcessConsoleSettingsPtr settings = recents->getExecutableConsoleSettings(process->getExecutableFile());
    if (settings)
      changeSecondComponent(process->createComponent(settings));
  }
}

juce::Component* ProcessManager::createComponent() const
  {return new ProcessManagerComponent(ProcessManagerPtr(const_cast<ProcessManager* >(this)));}
