/*-----------------------------------------.---------------------------------.
| Filename: NetworkInterface.cpp           | Network Interface               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Stream.h>

#include <lbcpp/Network/NetworkInterface.h>
#include "NetworkNotification.h"

using namespace lbcpp;

static void createDirectoryIfNotExists(ExecutionContext& context, const File& directory)
{
  if (!directory.exists())
    directory.createDirectory();
}

/*
** NetworkInterface
*/
void NetworkInterface::sendInterfaceClass()
  {client->sendVariable(getClassName());}
    
void NetworkInterface::closeCommunication(ExecutionContext& context)
  {client->stopClient();}

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
  return new NetworkResponse(context, ExecutionTrace::createFromFile(context, f));
}

/*
** NodeNetworkInterface - ManagerNodeNetworkInterface
*/
ManagerNodeNetworkInterface::ManagerNodeNetworkInterface(ExecutionContext& context)
  : NodeNetworkInterface(context, T("manager")),
    requestDirectory(context.getFile(T("Requests"))),
    archiveDirectory(context.getFile(T("Archives")))
{
  createDirectoryIfNotExists(context, requestDirectory);
  createDirectoryIfNotExists(context, archiveDirectory);

  StreamPtr files = directoryFileStream(context, requestDirectory, T("*.request"));
  while (!files->isExhausted())
  {
    NetworkRequestPtr request = NetworkRequest::createFromFile(context, files->next().getFile());
    context.informationCallback(T("Request restored: ") + request->getWorkUnitInformation()->getIdentifier());
    requests.push_back(request);
  }
}

void ManagerNodeNetworkInterface::closeCommunication(ExecutionContext& context)
  {client->stopClient();}

WorkUnitInformationPtr ManagerNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request)
{
  WorkUnitInformationPtr res = request->getWorkUnitInformation();
  res->selfGenerateIdentifier();
  res->setStatus(WorkUnitInformation::waitingOnManager);
  /* First, backup request */
  request->saveToFile(context, requestDirectory.getChildFile(res->getIdentifier() + T(".request")));
  requests.push_back(request);
  return res;
}

WorkUnitInformation::Status ManagerNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const
{
  // Seek in Request directory
  File f = requestDirectory.getChildFile(information->getIdentifier() + T(".request"));
  if (f.exists())
    return NetworkRequest::createFromFile(context, f).staticCast<NetworkRequest>()->getWorkUnitInformation()->getStatus();
  // Otherwise, maybe in Archive directory
  f = archiveDirectory.getChildFile(information->getIdentifier() + T(".request"));
  if (f.exists())
    return NetworkRequest::createFromFile(context, f).staticCast<NetworkRequest>()->getWorkUnitInformation()->getStatus();

  return WorkUnitInformation::iDontHaveThisWorkUnit;
}

NetworkResponsePtr ManagerNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const
{
  File f = archiveDirectory.getChildFile(information->getIdentifier() + T(".response"));
  if (!f.exists())
    return NetworkResponsePtr();

  return NetworkResponse::createFromFile(context, f);
}

void ManagerNodeNetworkInterface::getUnfinishedRequestsSentTo(const String& nodeName, std::vector<NetworkRequestPtr>& results) const
{
  for (size_t i = 0; i < requests.size(); ++i)
  {
    if (requests[i]->getWorkUnitInformation()->getStatus() == WorkUnitInformation::finished)
      continue;
    if (requests[i]->getWorkUnitInformation()->getDestination() == nodeName)
      results.push_back(requests[i]);
  }
}

void ManagerNodeNetworkInterface::archiveTrace(ExecutionContext& context, const NetworkRequestPtr& request, const NetworkResponsePtr& trace)
{
  File f = requestDirectory.getChildFile(request->getWorkUnitInformation()->getIdentifier() + T(".request"));
  if (f.exists())
    f.deleteFile();

  f = archiveDirectory.getChildFile(request->getWorkUnitInformation()->getIdentifier() + T(".request"));
  request->saveToFile(context, f);

  f = archiveDirectory.getChildFile(request->getWorkUnitInformation()->getIdentifier() + T(".response"));
  trace->saveToFile(context, f);

  std::vector<NetworkRequestPtr> res;
  res.reserve(requests.size() - 1);
  for (size_t i = 0; i < requests.size(); ++i)
    if (requests[i]->getWorkUnitInformation()->getStatus() != WorkUnitInformation::finished)
      res.push_back(requests[i]);
  requests = res;
}

