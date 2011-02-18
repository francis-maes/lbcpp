/*-----------------------------------------.---------------------------------.
| Filename: ExplorerProject.h              | Project                         |
| Author  : Francis Maes                   |                                 |
| Started : 18/01/2011 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_PROJECT_H_
# define LBCPP_EXPLORER_PROJECT_H_

# include <lbcpp/lbcpp.h>
# include "ExplorerConfiguration.h"

namespace lbcpp
{

class RecentWorkUnitConfiguration : public Object
{
public:
  RecentWorkUnitConfiguration(const String& workUnitName);
  RecentWorkUnitConfiguration() {}

  void addRecentArguments(const String& args);
  void addRecentWorkingDirectory(const File& workingDirectory);

  const String& getWorkUnitName() const
    {return workUnitName;}

  const std::vector<String>& getArguments() const
    {return arguments;}

  const std::vector<File>& getWorkingDirectories() const
    {return workingDirectories;}

  WorkUnitPtr createWorkUnit() const
  {
    WorkUnitPtr res = WorkUnit::create(getType(workUnitName));
    if (!res)
      defaultExecutionContext().errorCallback(T("Could not create work unit of class ") + workUnitName);
    return res;
  }

protected:
  friend class RecentWorkUnitConfigurationClass;

  String workUnitName;
  std::vector<String> arguments;
  std::vector<File> workingDirectories;
};

typedef ReferenceCountedObjectPtr<RecentWorkUnitConfiguration> RecentWorkUnitConfigurationPtr;

class RecentWorkUnitsConfiguration : public Object
{
public:
  virtual String getName() const
    {return T("RecentWorkUnitsConfiguration");}

  size_t getNumRecentWorkUnits() const
    {return recents.size();}
  
  RecentWorkUnitConfigurationPtr getRecentWorkUnit(size_t index) const
    {return recents[index];}
  RecentWorkUnitConfigurationPtr getWorkUnit(const String& name);

  void addRecentWorkUnit(const String& workUnitName);
  void addRecent(const String& workUnitName, const String& arguments, const File& workingDirectory);

  void clear()
    {recents.clear();}

protected:
  friend class RecentWorkUnitsConfigurationClass;

  std::vector<RecentWorkUnitConfigurationPtr> recents;

  int findRecentWorkUnit(const String& workUnit) const;
};

typedef ReferenceCountedObjectPtr<RecentWorkUnitsConfiguration> RecentWorkUnitsConfigurationPtr;

class ExplorerProject;
typedef ReferenceCountedObjectPtr<ExplorerProject> ExplorerProjectPtr;

class ExplorerProject : public Object
{
public:
  ExplorerProject() : recentWorkUnits(new RecentWorkUnitsConfiguration())
  {
    workUnitContext = multiThreadedExecutionContext(juce::SystemStats::getNumCpus());
  }

  static ExplorerProjectPtr createProject(ExecutionContext& context, const File& rootDirectory);
  static ExplorerProjectPtr openProject(ExecutionContext& context, const File& rootDirectory);

  void save(ExecutionContext& context);
  void close(ExecutionContext& context);

  void setRootDirectory(const File& rootDirectory)
  {
    this->rootDirectory = rootDirectory;
    workUnitContext->setProjectDirectory(rootDirectory);
  }

  const File& getRootDirectory() const
    {return rootDirectory;}

  const File& getRecentDirectory() const
    {return recentDirectory;}

  void setRecentDirectory(const File& recentDirectory)
    {this->recentDirectory = recentDirectory;}

  const RecentWorkUnitsConfigurationPtr& getRecentWorkUnits() const
    {return recentWorkUnits;}

  bool startWorkUnit(ExecutionContext& context, WorkUnitPtr& workUnit);

  ExecutionContextPtr workUnitContext;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExplorerProjectClass;

  File rootDirectory;
  File recentDirectory;
  RecentWorkUnitsConfigurationPtr recentWorkUnits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROJECT_H_
