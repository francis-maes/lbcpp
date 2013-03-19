/*-----------------------------------------.---------------------------------.
| Filename: RecentProcesses.h              | Recent Processes                |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_PROCESS_MANAGER_RECENTS_H_
# define EXPLORER_PROCESS_MANAGER_RECENTS_H_

# include "ProcessManager.h"
# include "../ExplorerConfiguration.h"

namespace lbcpp
{

class RecentProcesses;
typedef ReferenceCountedObjectPtr<RecentProcesses> RecentProcessesPtr;

class RecentProcesses : public Object
{
public:
  virtual string toShortString() const
    {return T("RecentProcesses");}

  static RecentProcessesPtr getInstance(ExecutionContext& context = defaultExecutionContext())
    {return RecentProcessesPtr();} // broken

  size_t getNumRecentExecutables() const
    {return v.size();}

  juce::File getRecentExecutable(size_t index) const
    {jassert(index < v.size()); return v[index].executable;}

  std::vector<string> getRecentArguments(const juce::File& executable) const;
  std::vector<juce::File> getRecentWorkingDirectories(const juce::File& executable) const;
  ProcessConsoleSettingsPtr getExecutableConsoleSettings(const juce::File& executable) const;

  void addRecentExecutable(const juce::File& file);
  void addRecent(const juce::File& executable, const string& arguments, const juce::File& workingDirectory);

  void clear()
    {v.clear();}

protected:
  struct RecentExecutable
  {
    RecentExecutable() {}
    RecentExecutable(const juce::File& executable);

    juce::File executable;
    std::vector<string> arguments;
    std::vector<juce::File> workingDirectories;
    ProcessConsoleSettingsPtr consoleSettings;

    void addRecentArguments(const string& args);
    void addRecentWorkingDirectory(const juce::File& workingDirectory);
  };

  std::vector<RecentExecutable> v;

  int findRecentExecutable(const juce::File& file) const;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_PROCESS_MANAGER_RECENTS_H_
