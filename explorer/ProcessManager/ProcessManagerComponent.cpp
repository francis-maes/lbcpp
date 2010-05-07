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
  ProcessConsoleComponent(ProcessPtr process) : process(process) {}

  juce_UseDebuggingNewOperator

private:
  ProcessPtr process;
};

class ProcessManagerListTabs : public TabbedComponent
{
public:
  ProcessManagerListTabs(ProcessManagerPtr processManager) : TabbedComponent(TabbedButtonBar::TabsAtBottom)
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

  juce_UseDebuggingNewOperator

private:
  void addProcessList(const String& name, ProcessListPtr processes)
    {addTab(name, Colours::lightblue, new ObjectContainerNameListComponent(processes), true);}
};

/*
** ProcessManagerComponent
*/
ProcessManagerComponent::ProcessManagerComponent(ProcessManagerPtr processManager)
  : SplittedLayout(new ProcessManagerListTabs(processManager), new Viewport(), 0.33, SplittedLayout::typicalHorizontal), processManager(processManager)
  {startTimer(100);}

void ProcessManagerComponent::timerCallback()
{
  processManager->updateProcesses();
  updateProcessLists();
}

void ProcessManagerComponent::updateProcessLists()
{
  ((ProcessManagerListTabs* )first)->updateContent();
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
      if (NewProcessDialogWindow::run(executable, arguments, workingDirectory) &&
          processManager->addNewProcess(executable, arguments, workingDirectory))
      {
        RecentProcessesPtr recent = RecentProcesses::getInstance();
        recent->addRecent(executable, arguments, workingDirectory);
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

juce::Component* ProcessManager::createComponent() const
  {return new ProcessManagerComponent(ProcessManagerPtr(const_cast<ProcessManager* >(this)));}
