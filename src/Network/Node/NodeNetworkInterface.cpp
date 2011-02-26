/*-----------------------------------------.---------------------------------.
| Filename: NodeNetworkInterface.cpp       | Node Network Interface          |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Stream.h>

#include "NodeNetworkInterface.h"
#include "NodeNetworkNotification.h"

using namespace lbcpp;

static inline void createDirectoryIfNotExists(ExecutionContext& context, const File& directory)
{
  if (!directory.exists())
    directory.createDirectory();
}

static inline File getArchiveFile(ExecutionContext& context, WorkUnitInformationPtr info)
{
  Time time(info->getCreationTime());
  return context.getFile(info->getProjectName() + T("/") + time.formatted(T("Y-m-d")) + T("/") + info->getIdentifier() + T(".archive"));
}

static inline File getRequestFile(ExecutionContext& context, WorkUnitInformationPtr info)
{
  return context.getFile(info->getProjectName() + T("/Requests/") + info->getIdentifier() + T(".request"));
}

static inline File getFailRequestFile(ExecutionContext& context, WorkUnitInformationPtr info)
{
  return context.getFile(info->getProjectName() + T("/Fail/") + info->getIdentifier() + T(".archive"));
}

static inline File createFullPathOfFile(const File& f)
{
  f.getParentDirectory().createDirectory();
  f.create();
  return f;
}

/*
** NodeNetworkInterface - ClientNodeNetworkInterface
*/
ClientNodeNetworkInterface::ClientNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
  : NodeNetworkInterface(context, client, nodeName) {}

void ClientNodeNetworkInterface::closeCommunication(ExecutionContext& context)
{
  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();
  return;
}

String ClientNodeNetworkInterface::getNodeName(ExecutionContext& context) const
{
  client->sendVariable(new GetNodeNameNotification());
  String res;
  if (!client->receiveString(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getNodeName"));
  return res;
}

WorkUnitInformationPtr ClientNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request)
{
  client->sendVariable(new PushWorkUnitNotification(request));
  WorkUnitInformationPtr res;
  if (!client->receiveObject<WorkUnitInformation>(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::pushWorkUnit"));
  return res;
}

WorkUnitInformation::Status ClientNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const
{
  client->sendVariable(new GetWorkUnitStatusNotification(information));
  int res = (int)WorkUnitInformation::communicationError;
  if (!client->receiveInteger(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getWorkUnitStatut"));
  return (WorkUnitInformation::Status)res;
}

NetworkResponsePtr ClientNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const
{
  client->sendVariable(new GetExecutionTraceNotification(information));
  NetworkResponsePtr res;
  if (!client->receiveObject<NetworkResponse>(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getExecutionTrace"));
  return res;
}

ObjectVectorPtr ClientNodeNetworkInterface::getModifiedStatusSinceLastConnection(ExecutionContext& context) const
{
  client->sendVariable(new GetModifiedStatusSinceLastConnection());
  ObjectVectorPtr res;
  if (!client->receiveObject<ObjectVector>(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getModifiedStatusSinceLastConnection"));
  return res;
}

/*
** NodeNetworkInterface - SgeNodeNetworkInterface
*/
SgeNodeNetworkInterface::SgeNodeNetworkInterface(ExecutionContext& context, const String& nodeName)
  : NodeNetworkInterface(context, nodeName)
{
  createDirectoryIfNotExists(context, context.getFile(T("Requests")));
  createDirectoryIfNotExists(context, context.getFile(T("Waiting")));
  createDirectoryIfNotExists(context, context.getFile(T("InProgress")));
  createDirectoryIfNotExists(context, context.getFile(T("Finished")));
  createDirectoryIfNotExists(context, context.getFile(T("Traces")));
  createDirectoryIfNotExists(context, context.getFile(T("ModifiedStatus")));
}

WorkUnitInformationPtr SgeNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request)
{
  WorkUnitInformationPtr res = request->getWorkUnitInformation();
  /* Check the validity of the work unit */
  WorkUnitPtr workUnit = request->getWorkUnit(context);
  if (!workUnit)
  {
    res->setStatus(WorkUnitInformation::workUnitError);
    return res;
  }

  res->setStatus(WorkUnitInformation::waitingOnServer);

  File f = context.getFile(T("Requests/") + request->getWorkUnitInformation()->getIdentifier() + T(".request"));
  res->saveToFile(context, f);

  f = context.getFile(T("Waiting/") + request->getWorkUnitInformation()->getIdentifier() + T(".workUnit"));
  workUnit->saveToFile(context, f);

  return res;
}

WorkUnitInformation::Status SgeNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const
{
  File f = context.getFile(T("Waiting/") + information->getIdentifier() + T(".workUnit"));
  if (f.exists())
    return WorkUnitInformation::waitingOnServer;

  f = context.getFile(T("InProgress/") + information->getIdentifier() + T(".workUnit"));
  if (f.exists())
    return WorkUnitInformation::running;

  f = context.getFile(T("Finished/") + information->getIdentifier() + T(".workUnit"));
  if (f.exists())
    return WorkUnitInformation::finished;

  return WorkUnitInformation::iDontHaveThisWorkUnit;
}

NetworkResponsePtr SgeNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const
{
  File f = context.getFile(T("Traces/") + information->getIdentifier() + T(".trace"));
  if (!f.exists())
    return NetworkResponsePtr();
  NetworkResponsePtr res = new NetworkResponse(context, ExecutionTrace::createFromFile(context, f));
  f.deleteFile();
  f = context.getFile(T("Requests/") + information->getIdentifier() + T(".request"));
  f.deleteFile();
  return res;
}

ObjectVectorPtr SgeNodeNetworkInterface::getModifiedStatusSinceLastConnection(ExecutionContext& context) const
{
  ObjectVectorPtr res = objectVector(workUnitInformationClass, 0);
  StreamPtr files = directoryFileStream(context, context.getProjectDirectory().getChildFile(T("ModifiedStatus")), T("*.status"));
  while (!files->isExhausted())
  {
    File f = files->next().getFile();
    String identifier = f.getFileNameWithoutExtension();
    NetworkRequestPtr request = NetworkRequest::createFromFile(context, context.getFile(T("Requests/") + identifier + T(".request")));
    if (!request)
      context.getFile(T("Requests/") + identifier + T(".request")).deleteFile();
    else
    {
      WorkUnitInformationPtr info = request->getWorkUnitInformation();
      info->setStatus(getWorkUnitStatus(context, info));
      res->append(info);
    }
    f.deleteFile();
  }
  return res;
}

/*
** NodeNetworkInterface - ManagerNodeNetworkInterface
*/
ManagerNodeNetworkInterface::ManagerNodeNetworkInterface(ExecutionContext& context)
  : NodeNetworkInterface(context, T("manager"))
{
  juce::OwnedArray<File> projectDirectories;
  context.getProjectDirectory().findChildFiles(projectDirectories, File::findDirectories, false, T("*"));

  for (size_t i = 0; i < (size_t)projectDirectories.size(); ++i)
  {
    context.enterScope(T("Loading project: ") + projectDirectories[i]->getFileName());
    StreamPtr files = directoryFileStream(context, projectDirectories[i]->getChildFile(T("Requests")), T("*.request"));
    while (!files->isExhausted())
    {
      File f = files->next().getFile();
      NetworkRequestPtr request = NetworkRequest::createFromFile(context, f);
      if (!request)
      {
        context.errorCallback(T("ManagerNodeNetworkInterface::ManagerNodeNetworkInterface"), T("Fail to restore: ") + f.getFileName());
        continue;
      }
      context.informationCallback(T("Request restored: ") + f.getFileNameWithoutExtension());
      addRequest(request);
    }
    context.leaveScope(Variable());
  }
}

void ManagerNodeNetworkInterface::addRequest(NetworkRequestPtr request)
{
  WorkUnitInformationPtr info = request->getWorkUnitInformation();
  if (!requests.count(info->getProjectName()))
    requests[info->getProjectName()] = std::map<String, NetworkRequestPtr>();
  requests[info->getProjectName()][info->getIdentifier()] = request;
}

void ManagerNodeNetworkInterface::removeRequest(WorkUnitInformationPtr info)
{
  if (!requests.count(info->getProjectName()))
    return;
  requests[info->getProjectName()].erase(info->getIdentifier());
}

NetworkRequestPtr ManagerNodeNetworkInterface::getRequest(WorkUnitInformationPtr info) const
{
  if (!requests.count(info->getProjectName()))
    return NetworkRequestPtr();
  std::map<String, NetworkRequestPtr> subMap = requests.find(info->getProjectName())->second;
  if (!subMap.count(info->getIdentifier()))
    return NetworkRequestPtr();
  return subMap.find(info->getIdentifier())->second;
}

void ManagerNodeNetworkInterface::closeCommunication(ExecutionContext& context)
  {client->stopClient();}

WorkUnitInformationPtr ManagerNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request)
{
  WorkUnitInformationPtr res = request->getWorkUnitInformation();
  res->selfGenerateIdentifier();
  res->setStatus(WorkUnitInformation::waitingOnManager);
  // backup request
  saveRequest(context, request);
  addRequest(request);
  return res;
}

WorkUnitInformation::Status ManagerNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr info) const
{
  File f = getArchiveFile(context, info);
  if (f.exists())
    return WorkUnitInformation::finished;

  f = getRequestFile(context, info);
  if (f.exists())
  {
    NetworkRequestPtr request = getRequest(info);
    if (!request)
    {
      context.errorCallback(T("ManagerNodeNetworkInterface::getWorkUnitStatus"), T("The request (" + info->getProjectName() + T("/") + info->getIdentifier() + ") has file but has not entry in the map."));
      return WorkUnitInformation::iDontHaveThisWorkUnit;
    }
    return request->getWorkUnitInformation()->getStatus();
  }
  return WorkUnitInformation::iDontHaveThisWorkUnit;
}

NetworkResponsePtr ManagerNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr info) const
{
  File f = getArchiveFile(context, info);
  if (!f.exists())
    return NetworkResponsePtr();

  NetworkArchivePtr archive = NetworkArchive::createFromFile(context, f);
  return archive->getNetworkResponse();
}

void ManagerNodeNetworkInterface::getRequestsWithStatus(const String& nodeName, WorkUnitInformation::Status status, std::vector<NetworkRequestPtr>& results) const
{
  std::map<String, std::map<String, NetworkRequestPtr> >::const_iterator i;
  for (i = requests.begin(); i != requests.end(); ++i)
  {
    std::map<String, NetworkRequestPtr>::const_iterator j;
    for (j = i->second.begin(); j != i->second.end(); ++j)
    {
      WorkUnitInformationPtr info = (j->second)->getWorkUnitInformation();
      if (info->getDestination() == nodeName && info->getStatus() == status)
        results.push_back(j->second);
    }
  }
}

void ManagerNodeNetworkInterface::archiveRequest(ExecutionContext& context, NetworkRequestPtr request, NetworkResponsePtr response)
{
  File f = getArchiveFile(context, request->getWorkUnitInformation());
  NetworkArchivePtr archive = new NetworkArchive(request, response);
  archive->saveToFile(context, createFullPathOfFile(f));
  
  f = getRequestFile(context, request->getWorkUnitInformation());
  f.deleteFile();

  removeRequest(request->getWorkUnitInformation());
}

void ManagerNodeNetworkInterface::setRequestAsFail(ExecutionContext& context, NetworkRequestPtr request, NetworkResponsePtr response)
{
  File f = getFailRequestFile(context, request->getWorkUnitInformation());
  NetworkArchivePtr archive = new NetworkArchive(request, response);
  archive->saveToFile(context, createFullPathOfFile(f));
  
  f = getRequestFile(context, request->getWorkUnitInformation());
  f.deleteFile();
  
  removeRequest(request->getWorkUnitInformation());
}

void ManagerNodeNetworkInterface::saveRequest(ExecutionContext& context, NetworkRequestPtr request) const
{
  request->saveToFile(context, createFullPathOfFile(getRequestFile(context, request->getWorkUnitInformation())));
}

ObjectVectorPtr ManagerNodeNetworkInterface::getModifiedStatusSinceLastConnection(ExecutionContext& context) const
{
  return ObjectVectorPtr();
}
