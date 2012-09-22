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
  virtual String getName() const
    {return T("RecentProcesses");}

  static RecentProcessesPtr getInstance(ExecutionContext& context = defaultExecutionContext())
    {return RecentProcessesPtr();} // broken

  size_t getNumRecentExecutables() const
    {return v.size();}

  File getRecentExecutable(size_t index) const
    {jassert(index < v.size()); return v[index].executable;}

  std::vector<String> getRecentArguments(const File& executable) const;
  std::vector<File> getRecentWorkingDirectories(const File& executable) const;
  ProcessConsoleSettingsPtr getExecutableConsoleSettings(const File& executable) const;

  void addRecentExecutable(const File& file);
  void addRecent(const File& executable, const String& arguments, const File& workingDirectory);

  void clear()
    {v.clear();}

protected:
  struct RecentExecutable
  {
    RecentExecutable() {}
    RecentExecutable(const File& executable);

    File executable;
    std::vector<String> arguments;
    std::vector<File> workingDirectories;
    ProcessConsoleSettingsPtr consoleSettings;

    void addRecentArguments(const String& args);
    void addRecentWorkingDirectory(const File& workingDirectory);
  };

  std::vector<RecentExecutable> v;

  int findRecentExecutable(const File& file) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_RECENTS_H_
