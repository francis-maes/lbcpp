/*-----------------------------------------.---------------------------------.
| Filename: ExplorerProject.h              | Project                         |
| Author  : Francis Maes                   |                                 |
| Started : 18/01/2011 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_PROJECT_H_
# define EXPLORER_PROJECT_H_

# include "ExplorerConfiguration.h"
# include <oil/Execution/WorkUnit.h>

namespace lbcpp
{

class RecentWorkUnitConfiguration : public Object
{
public:
  RecentWorkUnitConfiguration(const string& workUnitName);
  RecentWorkUnitConfiguration() {}

  void addRecentArguments(const string& args);

  const string& getWorkUnitName() const
    {return workUnitName;}

  const std::vector<string>& getArguments() const
    {return arguments;}

  WorkUnitPtr createWorkUnit() const
  {
    WorkUnitPtr res = WorkUnit::create(getType(workUnitName));
    if (!res)
      defaultExecutionContext().errorCallback(T("Could not create work unit of class ") + workUnitName);
    return res;
  }

protected:
  friend class RecentWorkUnitConfigurationClass;

  string workUnitName;
  std::vector<string> arguments;
};

typedef ReferenceCountedObjectPtr<RecentWorkUnitConfiguration> RecentWorkUnitConfigurationPtr;

class RecentWorkUnitsConfiguration : public Object
{
public:
  string toShortString() const
    {return T("RecentWorkUnitsConfiguration");}

  size_t getNumRecentWorkUnits() const
    {return recents.size();}
  
  RecentWorkUnitConfigurationPtr getRecentWorkUnit(size_t index) const
    {return recents[index];}
  RecentWorkUnitConfigurationPtr getWorkUnit(const string& name);

  void addRecentWorkUnit(const string& workUnitName);
  void addRecent(const string& workUnitName, const string& arguments);

  void clear()
    {recents.clear();}

protected:
  friend class RecentWorkUnitsConfigurationClass;

  std::vector<RecentWorkUnitConfigurationPtr> recents;

  int findRecentWorkUnit(const string& workUnit) const;
};

typedef ReferenceCountedObjectPtr<RecentWorkUnitsConfiguration> RecentWorkUnitsConfigurationPtr;

class ExplorerProject;
typedef ReferenceCountedObjectPtr<ExplorerProject> ExplorerProjectPtr;

class ExplorerProject : public Object
{
public:
  ExplorerProject();

  /*
  ** Current project
  */
  static ExplorerProjectPtr currentProject;

  static ExplorerProjectPtr getCurrentProject()
    {return currentProject;}

  static bool hasCurrentProject()
    {return currentProject;}

  /*
  ** Create / Open / Save / Close
  */
  static ExplorerProjectPtr createProject(ExecutionContext& context, const juce::File& rootDirectory);
  static ExplorerProjectPtr openProject(ExecutionContext& context, const juce::File& rootDirectory);

  void save(ExecutionContext& context);
  void close(ExecutionContext& context);

  virtual string toShortString() const
    {return rootDirectory.getFileName();}

  /*
  ** Directories
  */
  void setRootDirectory(const juce::File& rootDirectory)
  {
    this->rootDirectory = rootDirectory;
    workUnitContext->setProjectDirectory(rootDirectory);
  }

  const juce::File& getRootDirectory() const
    {return rootDirectory;}

  const juce::File& getRecentDirectory() const
    {return recentDirectory;}

  void setRecentDirectory(const juce::File& recentDirectory)
    {this->recentDirectory = recentDirectory;}

  /*
  ** Work Units
  */
  const RecentWorkUnitsConfigurationPtr& getRecentWorkUnits() const
    {return recentWorkUnits;}

  bool startWorkUnit(ExecutionContext& context, WorkUnitPtr& workUnit);

  ExecutionContextPtr workUnitContext;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExplorerProjectClass;

  juce::File rootDirectory;
  juce::File recentDirectory;
  RecentWorkUnitsConfigurationPtr recentWorkUnits;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_PROJECT_H_
