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
  Process(const File& executableFile, const String& arguments, const File& workingDirectory, const String& name = String::empty)
    : NameableObject(name.isEmpty() ? executableFile.getFileNameWithoutExtension() + T(" ") + arguments : name),
      executableFile(executableFile), arguments(arguments), workingDirectory(workingDirectory) {}

  virtual String toString() const
    {return executableFile.getFullPathName() + T(" ") + arguments;}

  virtual bool start() = 0;
  virtual bool kill() = 0;

  virtual void update() = 0;
  virtual bool isFinished() const = 0;
  
  const std::vector<String>& getProcessOutput() const
    {return processOutput;}

protected:
  File executableFile;
  String arguments;
  File workingDirectory;

  std::vector<String> processOutput;
};

typedef ReferenceCountedObjectPtr<Process> ProcessPtr;

class ProcessList;
typedef ReferenceCountedObjectPtr<ProcessList> ProcessListPtr;

class ProcessList : public VectorObjectContainer
{
public:
  size_t getNumProcesses() const
    {return size();}

  ProcessPtr getProcess(size_t index) const
    {return getAndCast<Process>(index);}

  void moveToTop(size_t index, ProcessListPtr target)
  {
    ProcessPtr process = getProcess(index);
    objects.erase(objects.begin() + index);
    target->prepend(process);
  }

  void moveToBottom(size_t index, ProcessListPtr target)
  {
    ProcessPtr process = getProcess(index);
    objects.erase(objects.begin() + index);
    target->append(process);
  }
};

class ProcessManager : public Object
{
public:
  ProcessManager();

  virtual ~ProcessManager()
    {killAllRunningProcesses();}

  virtual String getName() const
    {return T("Process Manager");}

  virtual juce::Component* createComponent() const;

  virtual ProcessPtr addNewProcess(const File& executable, const String& arguments, const File& workingDirectory) = 0;
  virtual size_t getNumberOfCpus() const = 0;

  ProcessListPtr getRunningProcesses() const
    {return runningProcesses;}

  ProcessListPtr getWaitingProcesses() const
    {return waitingProcesses;}

  ProcessListPtr getFinishedProcesses() const
    {return finishedProcesses;}

  ProcessListPtr getKilledProcesses() const
    {return killedProcesses;}

  void updateProcesses();
  void clearFinishedProcessLists();
  void killAllRunningProcesses();

protected:
  ProcessListPtr runningProcesses;
  ProcessListPtr waitingProcesses;
  ProcessListPtr finishedProcesses;
  ProcessListPtr killedProcesses;
};

typedef ReferenceCountedObjectPtr<ProcessManager> ProcessManagerPtr;

extern ProcessManagerPtr localProcessManager();

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_H_
