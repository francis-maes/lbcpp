/*-----------------------------------------.---------------------------------.
| Filename: ExplorerProject.cpp            | Project                         |
| Author  : Francis Maes                   |                                 |
| Started : 18/01/2011 19:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ExplorerProject.h"
#include "WorkUnitManager/NewWorkUnitDialogWindow.h"

using namespace lbcpp;

ExplorerProjectPtr ExplorerProject::currentProject;

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

/*
** RecentWorkUnitsConfiguration
*/
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
  ExplorerConfiguration::save(defaultExecutionContext());
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

void RecentWorkUnitsConfiguration::addRecent(const String& workUnit, const String& arguments)
{
  addRecentWorkUnit(workUnit);
  recents[0]->addRecentArguments(arguments);
  ExplorerConfiguration::save(defaultExecutionContext());
}

int RecentWorkUnitsConfiguration::findRecentWorkUnit(const String& workUnit) const
{
  for (size_t i = 0; i < recents.size(); ++i)
    if (recents[i]->getWorkUnitName() == workUnit)
      return (int)i;
  return -1;
}

/*
** ExplorerProject
*/
ExplorerProject::ExplorerProject() : recentWorkUnits(new RecentWorkUnitsConfiguration())
{
  int numCpus = juce::SystemStats::getNumCpus();
  workUnitContext = multiThreadedExecutionContext(numCpus > 2 ? numCpus - 1 : numCpus);
//  workUnitContext->appendCallback(consoleExecutionCallback());
}

ExplorerProjectPtr ExplorerProject::createProject(ExecutionContext& context, const File& rootDirectory)
{
  ExplorerProjectPtr res(new ExplorerProject());
  if (rootDirectory.existsAsFile())
    rootDirectory.deleteFile();
  if (rootDirectory.exists())
    rootDirectory.deleteRecursively();
  if (!rootDirectory.createDirectory())
  {
    context.errorCallback(T("Could not create directory ") + rootDirectory.getFullPathName());
    return ExplorerProjectPtr();
  }
  res->setRootDirectory(res->recentDirectory = rootDirectory);
  res->save(context); // create initial "project.xml" file
  return res;
}

ExplorerProjectPtr ExplorerProject::openProject(ExecutionContext& context, const File& rootDirectory)
{
  if (rootDirectory.existsAsFile())
  {
    context.errorCallback(rootDirectory.getFullPathName() + T(" is a file, not a directory"));
    return ExplorerProjectPtr();
  }
  if (!rootDirectory.exists())
  {
    context.errorCallback(rootDirectory.getFullPathName() + T(" does not exists"));
    return ExplorerProjectPtr();
  }

  ExplorerProjectPtr res;

  File xmlFile = rootDirectory.getChildFile(T("project.xml"));
  if (xmlFile.existsAsFile())
    res = ExplorerProject::createFromFile(context, xmlFile);
  else
    context.warningCallback(xmlFile.getFullPathName() + T(" does not exists"));
  
  if (!res)
  {
    res = new ExplorerProject();
    res->setRootDirectory(res->recentDirectory = rootDirectory);
    res->save(context); // create initial "project.xml" file
  }
  else
    res->setRootDirectory(rootDirectory);
  return res;
}

void ExplorerProject::save(ExecutionContext& context)
{
  saveToFile(context, rootDirectory.getChildFile(T("project.xml")));
}

void ExplorerProject::close(ExecutionContext& context)
{
  save(context);
}

bool ExplorerProject::startWorkUnit(ExecutionContext& context, WorkUnitPtr& workUnit)
{
  String workUnitName;
  String arguments;
  if (!NewWorkUnitDialogWindow::run(context, recentWorkUnits, workUnitName, arguments))
    return false;

  recentWorkUnits->addRecent(workUnitName, arguments);
  save(context);

  TypePtr workUnitType = typeManager().getType(context, workUnitName);
  if (!workUnitType)
    return false;

  workUnit = Object::create(workUnitType);
  if (!workUnit)
  {
    context.errorCallback(T("Create Work Unit"), T("Could not create ") + workUnitName);
    return false;
  }

  if (!workUnit->parseArguments(context, arguments))
    return false;

  return true;
}
