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
#include "../../juce/ConsoleProcess.h"

class LocalProcess : public Process
{
public:
  LocalProcess(const File& executableFile, const String& arguments, const File& workingDirectory, const String& name = String::empty)
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
    File exe = getCopyFile(*context, executableFile);
    if (!exe.exists())
      executableFile.copyFileTo(exe);
    if (!exe.exists())
    {
      context->errorCallback(T("LocalProcess::start"), T("Could not copy executable"));
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
      String str;
      if (!process->readStandardOutput(str) || str.isEmpty())
        break;
      
      if (processOutput.empty())
        processOutput.push_back(String::empty);
      String* lastLine = &processOutput.back();
      for (int j = 0; j < str.length(); ++j)
      {
        if (str[j] == '\r')
          continue;
        if (str[j] == '\n')
        {
          processOutput.push_back(String::empty);
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

  File getCopyFile(ExecutionContext& context, const File& executable)
  {
    Time lastModificationTime = executable.getLastModificationTime();
    File applicationData = ExplorerConfiguration::getApplicationDataDirectory(context);
    String name = executable.getFileNameWithoutExtension();
    String date = lastModificationTime.toString(true, true, true, true);
    name += T("_");
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

  virtual ProcessPtr addNewProcess(const File& executable, const String& arguments, const File& workingDirectory)
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
  SshSgeProcess(const File& executableFile, const String& arguments, const File& workingDirectory, const String& name = String::empty)
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
