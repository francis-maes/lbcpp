/*-----------------------------------------.---------------------------------.
| Filename: ProcessManager.h               | Process Manager                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_PROCESS_MANAGER_H_
# define LBCPP_EXPLORER_PROCESS_MANAGER_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Process : public NameableObject
{
public:
  Process(const File& executableFile, const String& arguments, const String& name = String::empty)
    : NameableObject(name.isEmpty() ? executableFile.getFileNameWithoutExtension() + T(" ") + arguments : name),
      executableFile(executableFile), arguments(arguments) {}

  virtual String toString() const
    {return executableFile.getFullPathName() + T(" ") + arguments;}

private:
  File executableFile;
  String arguments;
};

typedef ReferenceCountedObjectPtr<Process> ProcessPtr;

class ProcessList : public VectorObjectContainer
{
public:
  size_t getNumProcesses() const
    {return size();}

  ProcessPtr getProcess(size_t index) const
    {return getAndCast<Process>(index);}
};

typedef ReferenceCountedObjectPtr<ProcessList> ProcessListPtr;

class ProcessManager : public Object
{
public:
  ProcessManager() : runningProcesses(new ProcessList()), 
        waitingProcesses(new ProcessList()), 
        finishedProcesses(new ProcessList()), 
        killedProcesses(new ProcessList())
  {
  }

  virtual String getName() const
    {return T("Process Manager");}

  virtual juce::Component* createComponent() const;

  virtual ProcessPtr addNewProcess(const File& executable, const String& arguments, const File& workingDirectory) = 0;

  ProcessListPtr getRunningProcesses() const
    {return runningProcesses;}

  ProcessListPtr getWaitingProcesses() const
    {return waitingProcesses;}

  ProcessListPtr getFinishedProcesses() const
    {return finishedProcesses;}

  ProcessListPtr getKilledProcesses() const
    {return killedProcesses;}

protected:
  ProcessListPtr runningProcesses;
  ProcessListPtr waitingProcesses;
  ProcessListPtr finishedProcesses;
  ProcessListPtr killedProcesses;
};

typedef ReferenceCountedObjectPtr<ProcessManager> ProcessManagerPtr;

class LocalProcessManager : public ProcessManager
{
public:
  virtual ProcessPtr addNewProcess(const File& executable, const String& arguments, const File& workingDirectory)
  {
    ProcessPtr res = new Process(executable, arguments);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_H_
