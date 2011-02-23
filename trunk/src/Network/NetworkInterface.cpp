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

void createDirectoryIfNotExists(ExecutionContext& context, const File& directory)
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

NetworkRequestPtr ClientNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request)
{
  client->sendVariable(new PushWorkUnitNotification(request));
  NetworkRequestPtr res;
  if (!client->receiveObject<NetworkRequest>(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::pushWorkUnit"));
  return res;
}

int ClientNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
{
  client->sendVariable(new GetWorkUnitStatusNotification(request));
  int res = NetworkRequest::communicationError;
  if (!client->receiveInteger(10000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getWorkUnitStatut"));
  return res;
}

ExecutionTracePtr ClientNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
{
  client->sendVariable(new GetExecutionTraceNotification(request));
  ExecutionTracePtr res;
  if (!client->receiveObject<ExecutionTrace>(10000, res))
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

NetworkRequestPtr SgeNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request)
{
  File f = context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));
  request->getNetworkRequest()->saveToFile(context, f);

  f = context.getFile(T("Waiting/") + request->getIdentifier() + T(".workUnit"));
  request->getWorkUnit()->saveToFile(context, f);

  return request->getNetworkRequest();
}

int SgeNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
{
  File f = context.getFile(T("Waiting/") + request->getIdentifier() + T(".workUnit"));
  if (f.exists())
    return NetworkRequest::waitingOnServer;

  f = context.getFile(T("InProgress/") + request->getIdentifier() + T(".workUnit"));
  if (f.exists())
    return NetworkRequest::running;

  f = context.getFile(T("Finished/") + request->getIdentifier() + T(".workUnit"));
  if (f.exists())
    return NetworkRequest::finished;

  return NetworkRequest::iDontHaveThisWorkUnit;
}

ExecutionTracePtr SgeNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
{
  File f = context.getFile(T("Traces/") + request->getIdentifier() + T(".trace"));
  if (!f.exists())
    return ExecutionTracePtr();
  
  return ExecutionTrace::createFromFile(context, f);
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
    context.informationCallback(T("Request restored: ") + request->toString());
    requests.push_back(request);
  }
}

void ManagerNodeNetworkInterface::closeCommunication(ExecutionContext& context)
  {client->stopClient();}

NetworkRequestPtr ManagerNodeNetworkInterface::pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request)
{
  request->selfGenerateIdentifier();
  request->setStatus(NetworkRequest::waitingOnManager);
  /* First, backup request */
  request->saveToFile(context, requestDirectory.getChildFile(request->getIdentifier() + T(".request")));

  requests.push_back(request);
  return request;
}

int ManagerNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
{
  // Seek in Request directory
  File f = requestDirectory.getChildFile(request->getIdentifier() + T(".request"));
  if (f.exists())
    return NetworkRequest::createFromFile(context, f).staticCast<NetworkRequest>()->getStatus();
  // Otherwise, maybe in Archive directory
  f = archiveDirectory.getChildFile(request->getIdentifier() + T(".request"));
  if (f.exists())
    return NetworkRequest::createFromFile(context, f).staticCast<NetworkRequest>()->getStatus();

  return NetworkRequest::iDontHaveThisWorkUnit;
}

ExecutionTracePtr ManagerNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
{
  File f = archiveDirectory.getChildFile(request->getIdentifier() + T(".trace"));
  if (!f.exists())
    return ExecutionTracePtr();
  return ExecutionTrace::createFromFile(context, f);
}

void ManagerNodeNetworkInterface::getUnfinishedRequestsSentTo(const String& nodeName, std::vector<WorkUnitNetworkRequestPtr>& results) const
{
  for (size_t i = 0; i < requests.size(); ++i)
  {
    if (requests[i]->getStatus() == NetworkRequest::finished)
      continue;
    if (requests[i]->getDestination() == nodeName)
      results.push_back(requests[i]);
  }
}

void ManagerNodeNetworkInterface::archiveTrace(ExecutionContext& context, const WorkUnitNetworkRequestPtr& request, const ExecutionTracePtr& trace)
{
  File f = requestDirectory.getChildFile(request->getIdentifier() + T(".request"));
  if (f.exists())
    f.deleteFile();

  f = archiveDirectory.getChildFile(request->getIdentifier() + T(".request"));
  request->saveToFile(context, f);

  f = archiveDirectory.getChildFile(request->getIdentifier() + T(".trace"));
  trace->saveToFile(context, f);

  std::vector<WorkUnitNetworkRequestPtr> res;
  res.reserve(requests.size() - 1);
  for (size_t i = 0; i < requests.size(); ++i)
    if (requests[i]->getStatus() != NetworkRequest::finished)
      res.push_back(requests[i]);
  requests = res;
}

