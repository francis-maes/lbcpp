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
ExplorerProject::ExplorerProject() : recentWorkUnits(new RecentWorkUnitsConfiguration()), managerConnected(false), managerHostName(T("localhost")), managerPort(1664)
{
  int numCpus = juce::SystemStats::getNumCpus();
  workUnitContext =  multiThreadedExecutionContext(numCpus > 1 ? numCpus - 1 : 1);
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

  // try to connect to manager if it was previously connected
  if (res->isManagerConnected())
    res->connectToManager(context, res->getManagerHostName(), res->getManagerPort());
  return res;
}

void ExplorerProject::save(ExecutionContext& context)
{
  saveToFile(context, rootDirectory.getChildFile(T("project.xml")));
}

void ExplorerProject::close(ExecutionContext& context)
{
  save(context);
  disconnectFromManager(context);
}

bool ExplorerProject::startWorkUnit(ExecutionContext& context, WorkUnitPtr& workUnit, String& targetGrid)
{
  String workUnitName;
  String arguments;
  targetGrid = recentTargetGrid;
  if (!NewWorkUnitDialogWindow::run(context, recentWorkUnits, workUnitName, arguments, targetGrid))
    return false;
  recentTargetGrid = targetGrid;

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

bool ExplorerProject::connectToManager(ExecutionContext& context, const String& hostName, int port)
{
  managerHostName = hostName;
  managerPort = port;
  managerConnected = true;

  managerClient = blockingNetworkClient(context);
  if (!managerClient->startClient(managerHostName, managerPort))
  {
    context.errorCallback(T("ExplorerProject::connectToManager"), T("Could not connect to manager at ") + managerHostName + T(":") + String(port));
    return false;
  }
  context.informationCallback(managerHostName, T("Connected !"));

  thisNetworkNodeName = T("lbcpp-explorer on ") + juce::SystemStats::getOperatingSystemName();
  managerInterface = clientManagerNetworkInterface(context, managerClient, thisNetworkNodeName);
  managerInterface->sendInterfaceClass();
  return true;
}

void ExplorerProject::disconnectFromManager(ExecutionContext& context)
{
  managerConnected = false;
  if (managerClient)
  {
    if (managerInterface)
    {
      managerInterface->closeCommunication();
      managerInterface = ManagerNetworkInterfacePtr();
    }
    managerClient->stopClient();
    managerClient = NetworkClientPtr();
  }
}

bool ExplorerProject::sendWorkUnitToManager(ExecutionContext& context, const WorkUnitPtr& workUnit, const String& grid, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
{
  if (!managerInterface)
  {
    context.errorCallback(T("Not connected to manager"));
    return false;
  }
  WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, getName(), thisNetworkNodeName, grid, workUnit, requiredCpus, requiredMemory, requiredTime);
  String res = managerInterface->pushWorkUnit(request);
  if (res == T("Error"))
  {
    context.errorCallback(T("Error while sending work unit to manager"));
    return false;
  }
  request->setIdentifier(res);
  context.informationCallback(T("WorkUnitIdentifier: ") + request->getIdentifier());
  return true;
}
