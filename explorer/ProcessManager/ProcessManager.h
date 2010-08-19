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

template<>
struct Traits<juce::Colour>
{
  typedef juce::Colour Type;

  static inline String toString(const juce::Colour& value)
    {return value.toString();}

  static inline void write(OutputStream& ostr, const juce::Colour& value)
    {ostr.writeString(value.toString());}

  static inline bool read(InputStream& istr, juce::Colour& res)
  {
    if (istr.isExhausted())
      return false;
    String str = istr.readString();
    res = juce::Colour::fromString(str);
    return true;
  }
};

class ProcessConsoleSettings;
typedef ReferenceCountedObjectPtr<ProcessConsoleSettings> ProcessConsoleSettingsPtr;

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

  File getExecutableFile() const
    {return executableFile;}

  String getArguments() const
    {return arguments;}

  File getWorkingDirectory() const
    {return workingDirectory;}

  virtual juce::Component* createComponent() const;
  juce::Component* createComponent(ProcessConsoleSettingsPtr settings) const;

protected:
  File executableFile;
  String arguments;
  File workingDirectory;

  std::vector<String> processOutput;
};

typedef ReferenceCountedObjectPtr<Process> ProcessPtr;

class ProcessConsoleFilter : public Object
{
public:
  ProcessConsoleFilter(const String& pattern = String::empty, const juce::Colour& colour = juce::Colours::red)
    : pattern(pattern), display(true), colour(colour) {}
  
  virtual juce::Component* createComponent() const;

  juce::Colour getColour() const
    {return colour;}

  bool match(const String& text) const
    {return pattern.isNotEmpty() && text.indexOf(pattern) >= 0;}

  void setDisplayFlag(bool display)
    {this->display = display;}

  bool getDisplayFlag() const
    {return display;}

  String getPattern() const
    {return pattern;}

  void setPattern(const String& pattern)
    {this->pattern = pattern;}

private:
  String pattern;
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

  juce::Colour getColourForLine(const String& line, bool& display) const
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

class ProcessList : public Vector
{
public:
  size_t getNumProcesses() const
    {return size();}

  ProcessPtr getProcess(size_t index) const
    {return getObjectAndCast<Process>(index);}

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
