/*-----------------------------------------.---------------------------------.
| Filename: WorkUnitManagerConfiguration.h | WorkUnit Manager Configuration  |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 03:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_WORK_UNIT_MANAGER_CONFIGURATION_H_
# define LBCPP_EXPLORER_WORK_UNIT_MANAGER_CONFIGURATION_H_

# include "../ExplorerConfiguration.h"

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

protected:
  friend class RecentWorkUnitConfigurationClass;

  String workUnitName;
  std::vector<String> arguments;
  std::vector<File> workingDirectories;
};

typedef ReferenceCountedObjectPtr<RecentWorkUnitConfiguration> RecentWorkUnitConfigurationPtr;


class RecentWorkUnitsConfiguration;
typedef ReferenceCountedObjectPtr<RecentWorkUnitsConfiguration> RecentWorkUnitsConfigurationPtr;

class RecentWorkUnitsConfiguration : public Object
{
public:
  virtual String getName() const
    {return T("RecentWorkUnitsConfiguration");}

  static RecentWorkUnitsConfigurationPtr getInstance(ExecutionContext& context = *silentExecutionContext);

  size_t getNumRecentWorkUnits() const
    {return recents.size();}
  
  RecentWorkUnitConfigurationPtr getRecentWorkUnit(size_t index) const
    {return recents[index];}
  RecentWorkUnitConfigurationPtr getWorkUnit(const String& name) const;

  void addRecentWorkUnit(const String& workUnitName);
  void addRecent(const String& workUnitName, const String& arguments, const File& workingDirectory);

  void clear()
    {recents.clear();}

protected:
  friend class RecentWorkUnitsConfigurationClass;

  std::vector<RecentWorkUnitConfigurationPtr> recents;

  int findRecentWorkUnit(const String& workUnit) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_WORK_UNIT_MANAGER_CONFIGURATION_H_
