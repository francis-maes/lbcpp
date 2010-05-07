/*-----------------------------------------.---------------------------------.
| Filename: ProcessManager.cpp             | Process Manager                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProcessManager.h"
using namespace lbcpp;

/*
** ProcessManager
*/
ProcessManager::ProcessManager() : runningProcesses(new ProcessList()), 
      waitingProcesses(new ProcessList()), 
      finishedProcesses(new ProcessList()), 
      killedProcesses(new ProcessList())
{
}

void ProcessManager::updateProcesses()
{
  // remove finished processes
  for (size_t i = 0; i < runningProcesses->size();)
    if (runningProcesses->getProcess(i)->isFinished())
      runningProcesses->moveToTop(i, finishedProcesses);
    else
      ++i;

  // add waiting processes
  size_t numCpus = getNumberOfCpus();
  while (runningProcesses->size() < numCpus && waitingProcesses->size() > 0)
  {
    ProcessPtr process = waitingProcesses->getProcess(0);
    waitingProcesses->moveToBottom(0, runningProcesses);
    process->start();
  }
}

void ProcessManager::clearFinishedProcessLists()
{
  finishedProcesses->clear();
  killedProcesses->clear();
}

void ProcessManager::killAllRunningProcesses()
{
  for (size_t i = 0; i < runningProcesses->size();)
  {
    ProcessPtr process = runningProcesses->getProcess(i);
    if (process->isFinished())
      runningProcesses->moveToTop(i, finishedProcesses);
    else
    {
      process->kill();
      runningProcesses->moveToTop(i, killedProcesses);
    }
  }
}

/*
** LocalProcessManager
*/
#include "../../juce/ConsoleProcess.h"

class LocalProcess : public Process
{
public:
  LocalProcess(const File& executableFile, const String& arguments, const File& workingDirectory, const String& name = String::empty)
    : Process(executableFile, arguments, workingDirectory, name), process(NULL) {}
 
  virtual void start()
  {
    jassert(!process);
    process = juce::ConsoleProcess::create(executableFile.getFullPathName(), arguments, workingDirectory.getFullPathName());
  }

  virtual void kill()
  {
    jassert(process);
    delete process;
  }

  virtual bool isFinished() const
    {int returnCode; return !process || !process->isRunning(returnCode);}

private:
  juce::ConsoleProcess* process;
};

class LocalProcessManager : public ProcessManager
{
public:
  virtual size_t getNumberOfCpus() const
    {return juce::SystemStats::getNumCpus();}

  virtual ProcessPtr addNewProcess(const File& executable, const String& arguments, const File& workingDirectory)
  {
    ProcessPtr res = new LocalProcess(executable, arguments, workingDirectory);
    waitingProcesses->append(res);
    return res;
  }
};

ProcessManagerPtr lbcpp::localProcessManager()
  {return new LocalProcessManager();}