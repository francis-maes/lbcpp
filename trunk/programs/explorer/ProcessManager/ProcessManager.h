/*-----------------------------------------.---------------------------------.
| Filename: ProcessManager.h               | Process Manager                 |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_PROCESS_MANAGER_H_
# define EXPLORER_PROCESS_MANAGER_H_

namespace lbcpp
{

class ProcessConsoleSettings;
typedef ReferenceCountedObjectPtr<ProcessConsoleSettings> ProcessConsoleSettingsPtr;

class Process : public NameableObject
{
public:
  Process(const juce::File& executableFile, const string& arguments, const juce::File& workingDirectory, const string& name = string::empty)
    : NameableObject(name.isEmpty() ? executableFile.getFileNameWithoutExtension() + T(" ") + arguments : name),
      executableFile(executableFile), arguments(arguments), workingDirectory(workingDirectory) {}

  virtual string toString() const
    {return executableFile.getFullPathName() + T(" ") + arguments;}

  virtual bool start() = 0;
  virtual bool killProcess() = 0;

  virtual void update() = 0;
  virtual bool isFinished() const = 0;
  
  const std::vector<string>& getProcessOutput() const
    {return processOutput;}

  juce::File getExecutableFile() const
    {return executableFile;}

  string getArguments() const
    {return arguments;}

  juce::File getWorkingDirectory() const
    {return workingDirectory;}

  virtual juce::Component* createComponent() const;
  juce::Component* createComponent(ProcessConsoleSettingsPtr settings) const;

  lbcpp_UseDebuggingNewOperator

protected:
  juce::File executableFile;
  string arguments;
  juce::File workingDirectory;

  std::vector<string> processOutput;
};

typedef ReferenceCountedObjectPtr<Process> ProcessPtr;

class ProcessConsoleFilter : public Object
{
public:
  ProcessConsoleFilter(const string& pattern = string::empty, const juce::Colour& colour = juce::Colours::red)
    : pattern(pattern), display(true), colour(colour) {}
  
  virtual juce::Component* createComponent() const;

  juce::Colour getColour() const
    {return colour;}

  bool match(const string& text) const
    {return pattern.isNotEmpty() && text.indexOf(pattern) >= 0;}

  void setDisplayFlag(bool display)
    {this->display = display;}

  bool getDisplayFlag() const
    {return display;}

  string getPattern() const
    {return pattern;}

  void setPattern(const string& pattern)
    {this->pattern = pattern;}

private:
  string pattern;
  bool display;
  juce::Colour colour;
};

typedef ReferenceCountedObjectPtr<ProcessConsoleFilter> ProcessConsoleFilterPtr;

class ProcessConsoleSettings : public Object
{
public:
  size_t getNumFilters() const
    {return filters.size();}

  ProcessConsoleFilterPtr getFilter(size_t index) const
    {jassert(index < filters.size()); return filters[index];}

  void addFilter(ProcessConsoleFilterPtr filter)
    {filters.push_back(filter);}

  virtual juce::Component* createComponent() const;

  juce::Colour getColourForLine(const string& line, bool& display) const
  {
    if (filters.empty())
      return juce::Colours::white;

    for (size_t i = 0; i < filters.size(); ++i)
      if (filters[i]->match(line))
      {
        display = filters[i]->getDisplayFlag();
        return filters[i]->getColour();
      }
    ProcessConsoleFilterPtr filter = filters.back();
    display = filter->getDisplayFlag();
    return filter->getColour();
  }

private:
  std::vector<ProcessConsoleFilterPtr> filters;
};

class ProcessList;
typedef ReferenceCountedObjectPtr<ProcessList> ProcessListPtr;

class ProcessList : public OVector
{
public:
  ProcessList() : OVector(objectClass, 0) {}

  size_t getNumProcesses() const
    {return getNumElements();}

  ProcessPtr getProcess(size_t index) const
    {return getElement(index).staticCast<Process>();}

  void moveToTop(size_t index, ProcessListPtr target)
  {
    ProcessPtr process = getProcess(index);
    remove(index);
    target->prepend(process);
  }

  void moveToBottom(size_t index, ProcessListPtr target)
  {
    ProcessPtr process = getProcess(index);
    remove(index);
    target->append(process);
  }
};

class ProcessManager : public Object
{
public:
  ProcessManager();

  virtual ~ProcessManager()
    {killAllRunningProcesses();}

  virtual string toShortString() const
    {return T("Process Manager");}

  virtual juce::Component* createComponent() const;

  virtual ProcessPtr addNewProcess(const juce::File& executable, const string& arguments, const juce::File& workingDirectory) = 0;
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

#endif // !EXPLORER_PROCESS_MANAGER_H_
