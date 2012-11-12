/*-----------------------------------------.---------------------------------.
| Filename: RecentProcesses.h              | Recent Processes                |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_PROCESS_MANAGER_RECENTS_H_
# define LBCPP_EXPLORER_PROCESS_MANAGER_RECENTS_H_

# include "ProcessManager.h"
# include "../ExplorerConfiguration.h"

namespace lbcpp
{

class RecentProcesses;
typedef ReferenceCountedObjectPtr<RecentProcesses> RecentProcessesPtr;

class RecentProcesses : public Object
{
public:
  virtual String toShortString() const
    {return T("RecentProcesses");}

  static RecentProcessesPtr getInstance(ExecutionContext& context = defaultExecutionContext())
    {return RecentProcessesPtr();} // broken

  size_t getNumRecentExecutables() const
    {return v.size();}

  juce::File getRecentExecutable(size_t index) const
    {jassert(index < v.size()); return v[index].executable;}

  std::vector<String> getRecentArguments(const juce::File& executable) const;
  std::vector<juce::File> getRecentWorkingDirectories(const juce::File& executable) const;
  ProcessConsoleSettingsPtr getExecutableConsoleSettings(const juce::File& executable) const;

  void addRecentExecutable(const juce::File& file);
  void addRecent(const juce::File& executable, const String& arguments, const juce::File& workingDirectory);

  void clear()
    {v.clear();}

protected:
  struct RecentExecutable
  {
    RecentExecutable() {}
    RecentExecutable(const juce::File& executable);

    juce::File executable;
    std::vector<String> arguments;
    std::vector<juce::File> workingDirectories;
    ProcessConsoleSettingsPtr consoleSettings;

    void addRecentArguments(const String& args);
    void addRecentWorkingDirectory(const juce::File& workingDirectory);
  };

  std::vector<RecentExecutable> v;

  int findRecentExecutable(const juce::File& file) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_RECENTS_H_
