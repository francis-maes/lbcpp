/*-----------------------------------------.---------------------------------.
| Filename: ProcessManager.cpp             | Process Manager                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

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
  for (size_t i = 0; i < runningProcesses->size();)
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
  for (size_t i = 0; i < waitingProcesses->size() && runningProcesses->size() < numCpus;)
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
  for (size_t i = 0; i < runningProcesses->size();)
  {
    ProcessPtr process = runningProcesses->getProcess(i);
    if (process->isFinished())
      runningProcesses->moveToTop(i, finishedProcesses);
    else
    {
      if (process->kill())
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
    jassert(!process);
    File exe = getCopyFile(executableFile);
    if (!exe.exists())
      executableFile.copyFileTo(exe);
    if (!exe.exists())
    {
      Object::error(T("LocalProcess::start"), T("Could not copy executable"));
      return false;
    }
    process = juce::ConsoleProcess::create(exe.getFullPathName(), arguments, workingDirectory.getFullPathName());
    return process != NULL;
  }

  virtual bool kill()
  {
    if (process->kill())
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

  File getCopyFile(const File& executable)
  {
    Time lastModificationTime = executable.getLastModificationTime();
    File applicationData = ExplorerConfiguration::getApplicationDataDirectory();
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

#if 0
#include "/usr/include/ne7ssh.h"

class SshConnection
{
public:
  SshConnection(String userName, String userPassword, String remoteHost, size_t remotePort)
    : userName(userName), userPassword(userPassword), remoteHost(remoteHost), remotePort(remotePort), retry(maximumRetry), channel(-1), ssh(new ne7ssh())
  {
    ssh->setOptions("aes192-cbc", "hmac-md5");
  }
  
  virtual ~SshConnection()
    {ssh->close(channel); delete ssh;}

private:
  bool initiateConnection()
  {
    while (channel < 0 && retry)
    {
      channel = ssh->connectWithPassword(remoteHost, remotePort, userName, userPassword);
      jassert(channel);
      
      if (channel >= 0 && !ssh->waitFor(channel, " $", 10))
      {
        clearConnection();
        retry--;
      }      
    }
    
    if (!retry)
      return false;
    retry = maximumRetry;
    return true;
  }
  
  void clearConnection()
  {ssh->close(channel); channel = -1;}
  
  bool checkSSHConnection()
  {
    initiateConnection();

    while (!ssh->send ("echo OK\n", channel) && retry)
    {
      clearConnection();
      initiateConnection();

      retry--;
    }

    if (!retry)
      return false;
    retry = maximumRetry;
    
    // wait responce
    if (!ssh->waitFor(channel, " $", 5))
    {
      clearConnection();
      return false;
    }

    // read result
    std::cout << "[SSH] " << ssh->read(channel) << std::endl;
  }
  
  enum {maximumRetry = 3};
  
  String userName;
  String userPassword;
  String remoteHost;
  size_t remotePort;
  size_t retry;
  int channel;
  
  ne7ssh* ssh;
};

class SshSgeProcess : public Process
{
  SshSgeProcess(const File& executableFile, const String& arguments, const File& workingDirectory, const String& name = String::empty)
  : Process(executableFile, arguments, workingDirectory, name), process(0) {}
  
  virtual bool start()
  {
    return false;
  }
  
  virtual bool kill()
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