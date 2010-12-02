/*-----------------------------------------.---------------------------------.
| Filename: WorkUnitManagerConfiguration.cpp| WorkUnit Manager Configuration |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 03:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "WorkUnitManagerConfiguration.h"
#include "../ExplorerConfiguration.h"
using namespace lbcpp;

/*
** RecentWorkUnitConfiguration
*/

RecentWorkUnitConfiguration::RecentWorkUnitConfiguration(const String& workUnitName)
  : workUnitName(workUnitName)
{
  arguments.push_back(T(" "));
}

void RecentWorkUnitConfiguration::addRecentArguments(const String& args)
{
  size_t i;
  for (i = 0; i < arguments.size(); ++i)
  if (arguments[i] == args)
    break;
  if (i < arguments.size())
    arguments.erase(arguments.begin() + i);
  arguments.insert(arguments.begin(), args);
}

void RecentWorkUnitConfiguration::addRecentWorkingDirectory(const File& workingDirectory)
{
  size_t i;
  for (i = 0; i < workingDirectories.size(); ++i)
  if (workingDirectories[i] == workingDirectory)
    break;
  if (i < workingDirectories.size())
    workingDirectories.erase(workingDirectories.begin() + i);
  workingDirectories.insert(workingDirectories.begin(), workingDirectory);
}

/*
** RecentWorkUnitsConfiguration
*/
RecentWorkUnitsConfigurationPtr RecentWorkUnitsConfiguration::getInstance(ExecutionContext& context)
{
  Variable& res = ExplorerConfiguration::getConfiguration(context, T("RecentWorkUnitsConfiguration"));
  if (!res.exists())
    res = Variable(new RecentWorkUnitsConfiguration());
  return res.getObjectAndCast<RecentWorkUnitsConfiguration>();
}

void RecentWorkUnitsConfiguration::addRecentWorkUnit(const String& workUnitName)
{
  int index = findRecentWorkUnit(workUnitName);
  if (index >= 0)
  {
    RecentWorkUnitConfigurationPtr r = recents[index];
    recents.erase(recents.begin() + index);
    recents.insert(recents.begin(), r);
  }
  else
    recents.insert(recents.begin(), new RecentWorkUnitConfiguration(workUnitName));
  ExplorerConfiguration::save(*silentExecutionContext);
}

RecentWorkUnitConfigurationPtr RecentWorkUnitsConfiguration::getWorkUnit(const String& name)
{
  if (name.isEmpty())
    return RecentWorkUnitConfigurationPtr();
  int index = findRecentWorkUnit(name);
  if (index < 0)
  {
    addRecentWorkUnit(name);
    index = 0;
  }
  return recents[index];
}

void RecentWorkUnitsConfiguration::addRecent(const String& workUnit, const String& arguments, const File& workingDirectory)
{
  addRecentWorkUnit(workUnit);
  recents[0]->addRecentArguments(arguments);
  if (workingDirectory.exists())
    recents[0]->addRecentWorkingDirectory(workingDirectory);
  ExplorerConfiguration::save(*silentExecutionContext);
}

int RecentWorkUnitsConfiguration::findRecentWorkUnit(const String& workUnit) const
{
  for (size_t i = 0; i < recents.size(); ++i)
    if (recents[i]->getWorkUnitName() == workUnit)
      return (int)i;
  return -1;
}
