/*-----------------------------------------.---------------------------------.
| Filename: ProcessManager.cpp             | Process Manager                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ProcessManager.h"
#include "../ExplorerConfiguration.h"
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
  for (size_t i = 0; i < runningProcesses->getNumElements();)
  {
    ProcessPtr process = runningProcesses->getProcess(i);
    process->update();
    if (process->isFinished())
      runningProcesses->moveToTop(i, finishedProcesses);
    else
      ++i;
  }

  // add waiting processes
  size_t numCpus = getNumberOfCpus();
  for (size_t i = 0; i < waitingProcesses->getNumElements() && runningProcesses->getNumElements() < numCpus;)
  {
    ProcessPtr process = waitingProcesses->getProcess(i);
    if (process->start())
      waitingProcesses->moveToBottom(0, runningProcesses);
    else
      ++i;
  }
}

void ProcessManager::clearFinishedProcessLists()
{
  finishedProcesses->clear();
  killedProcesses->clear();
}

void ProcessManager::killAllRunningProcesses()
{
  for (size_t i = 0; i < runningProcesses->getNumElements();)
  {
    ProcessPtr process = runningProcesses->getProcess(i);
    if (process->isFinished())
      runningProcesses->moveToTop(i, finishedProcesses);
    else
    {
      if (process->killProcess())
        runningProcesses->moveToTop(i, killedProcesses);
      else
        ++i;
    }
  }
}

/*
** LocalProcessManager
*/
#include "../../extern/juce/ConsoleProcess.h"

class LocalProcess : public Process
{
public:
  LocalProcess(const juce::File& executableFile, const string& arguments, const juce::File& workingDirectory, const string& name = string::empty)
    : Process(executableFile, arguments, workingDirectory, name), process(NULL) {}
  virtual ~LocalProcess()
  {
    if (process)
      delete process;
  }
 
  virtual bool start()
  {
    ExecutionContextPtr context = defaultConsoleExecutionContext();

    jassert(!process);
    juce::File exe = getCopyFile(*context, executableFile);
    if (!exe.exists())
      executableFile.copyFileTo(exe);
    if (!exe.exists())
    {
      context->errorCallback(JUCE_T("LocalProcess::start"), JUCE_T("Could not copy executable"));
      return false;
    }
    jassert(false); // FIXME
    process = NULL;//juce::ConsoleProcess::create(exe.getFullPathName(), arguments, workingDirectory.getFullPathName());
    return process != NULL;
  }

  virtual bool killProcess()
  {
    if (process->killProcess())
    {
      jassert(process);
      delete process;
      process = NULL;
      return true;
    }
    return false;
  }

  virtual void update()
  {
    if (!process)
      return;
    for (int i = 0; i < 10; ++i)
    {
      string str;
      if (!process->readStandardOutput(str) || str.isEmpty())
        break;
      
      if (processOutput.empty())
        processOutput.push_back(string::empty);
      string* lastLine = &processOutput.back();
      for (int j = 0; j < str.length(); ++j)
      {
        if (str[j] == '\r')
          continue;
        if (str[j] == '\n')
        {
          processOutput.push_back(string::empty);
          lastLine = &processOutput.back();
        }
        else
          *lastLine += str[j];
      }
    }
  }

  virtual bool isFinished() const
    {int returnCode; return !process || !process->isRunning(returnCode);}

private:
  juce::ConsoleProcess* process;

  juce::File getCopyFile(ExecutionContext& context, const juce::File& executable)
  {
    juce::Time lastModificationTime = executable.getLastModificationTime();
    juce::File applicationData = ExplorerConfiguration::getApplicationDataDirectory(context);
    string name = executable.getFileNameWithoutExtension();
    string date = lastModificationTime.toString(true, true, true, true);
    name += JUCE_T("_");
    name += date.replaceCharacter(' ', '_').replaceCharacter(':', '_');
    name += executable.getFileExtension();
    return applicationData.getChildFile(name);
  }
};

class LocalProcessManager : public ProcessManager
{
public:
  virtual size_t getNumberOfCpus() const
    {return juce::SystemStats::getNumCpus();}

  virtual ProcessPtr addNewProcess(const juce::File& executable, const string& arguments, const juce::File& workingDirectory)
  {
    ProcessPtr res = new LocalProcess(executable, arguments, workingDirectory);
    waitingProcesses->append(res);
    return res;
  }
};

ProcessManagerPtr lbcpp::localProcessManager()
  {return new LocalProcessManager();}

#if 1
class SshSgeProcess : public Process
{
  SshSgeProcess(const juce::File& executableFile, const string& arguments, const juce::File& workingDirectory, const string& name = string::empty)
  : Process(executableFile, arguments, workingDirectory, name), process(0) {}
  
  virtual bool start()
  {
    return false;
  }
  
  virtual bool killProcess()
  {
    return false;
  }
  
  virtual void update()
  {
    
  }
  
  virtual bool isFinished() const
  {
    return false;
  }
  
private:
  size_t process;
};
#endif
