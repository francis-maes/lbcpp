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

void createDirectoryIfNotExists(ExecutionContext& context, const String& directoryName)
{
  File f = context.getFile(directoryName);
  if (!f.exists())
    f.createDirectory();
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
  createDirectoryIfNotExists(context, T("Requests"));
  createDirectoryIfNotExists(context, T("Waiting"));
  createDirectoryIfNotExists(context, T("InProgress"));
  createDirectoryIfNotExists(context, T("Finished"));
  createDirectoryIfNotExists(context, T("Traces"));
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
  : NodeNetworkInterface(context, T("manager"))
{
  createDirectoryIfNotExists(context, T("Requests"));
  createDirectoryIfNotExists(context, T("WorkUnits"));
  createDirectoryIfNotExists(context, T("Traces"));
  
  StreamPtr files = directoryFileStream(context, context.getFile(T("Requests/")), T("*.request"));
  while (!files->isExhausted())
  {
    NetworkRequestPtr request = NetworkRequest::createFromFile(context, files->next().getFile());
    std::cout << request->toString() << std::endl;
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
  NetworkRequestPtr res = request->getNetworkRequest();
  File f = context.getFile(T("Requests/") + res->getIdentifier() + T(".request"));
  res->saveToFile(context, f);
  f = context.getFile(T("WorkUnits/") + res->getIdentifier() + T(".workUnit"));
  request->getWorkUnit()->saveToFile(context, f);
  
  requests.push_back(res);
  return res;
}

int ManagerNodeNetworkInterface::getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
{
  File f = context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));
  if (!f.exists())
    return NetworkRequest::iDontHaveThisWorkUnit;
  NetworkRequestPtr localRequest = NetworkRequest::createFromFile(context, f);
  return localRequest->getStatus();
}

ExecutionTracePtr ManagerNodeNetworkInterface::getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
{
  File f = context.getFile(T("Traces/") + request->getIdentifier() + T(".trace"));
  if (!f.exists())
    return ExecutionTracePtr();
  return ExecutionTrace::createFromFile(context, f);
}

void ManagerNodeNetworkInterface::getUnfinishedRequestsSentTo(const String& nodeName, std::vector<NetworkRequestPtr>& results) const
{
  for (size_t i = 0; i < requests.size(); ++i)
    if (requests[i]->getStatus() != NetworkRequest::finished && requests[i]->getDestination() == nodeName)
      results.push_back(requests[i]);
}

WorkUnitNetworkRequestPtr ManagerNodeNetworkInterface::getWorkUnit(ExecutionContext& context, NetworkRequestPtr request) const
{
  File f = context.getFile(T("WorkUnits/") + request->getIdentifier() + T(".workUnit"));
  WorkUnitPtr workUnit = WorkUnit::createFromFile(context, f);
  return new WorkUnitNetworkRequest(request, workUnit);
}
