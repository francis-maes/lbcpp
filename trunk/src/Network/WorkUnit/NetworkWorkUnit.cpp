/*-----------------------------------------.---------------------------------.
| Filename: NetworkWorkUnit.cpp            | Network Work Unit               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkServer.h>
#include <lbcpp/Network/NetworkNotification.h>
#include "NetworkWorkUnit.h"

using namespace lbcpp;

/*
** ManagerWorkUnit
*/
Variable ManagerWorkUnit::run(ExecutionContext& context)
{
  fileManager = new NetworkProjectFileManager(context);
  NetworkServerPtr server = new NetworkServer(context);
  if (!server->startServer(port))
  {
    context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port ") + String((int)port));
    return false;
  }
  
  while (true)
  {
    /* Accept client */
    NetworkClientPtr client = server->acceptClient(INT_MAX);
    if (!client)
      continue;

    String connectedHostName = client->getConnectedHostName();
    context.informationCallback(connectedHostName, T("Connected"));
    
    /* Which kind of connection ? */
    NetworkInterfacePtr remoteInterface;
    if (!client->receiveObject<NetworkInterface>(300000, remoteInterface) || !remoteInterface)
    {
      context.warningCallback(connectedHostName, T("Unknown NetworkInterface - Need to update Manager and/or Client? Invalid NetworkInterface?"));
      client->stopClient();
      continue;
    }

    context.informationCallback(connectedHostName, T("Node name: ") + remoteInterface->getName());
    context.informationCallback(connectedHostName, T("Interface: ") + remoteInterface->getClassName());

    ClassPtr type = remoteInterface->getClass();
    /* Strat communication (depending of the type) */
    NetworkInterfacePtr interface;
    if (type->inheritsFrom(forwarderManagerNetworkInterfaceClass))
    {
      interface = new FileSystemManagerNetworkInterface(context, T("Manager"), fileManager);
      serverCommunication(context, interface, client);
    }
    else if (type->inheritsFrom(gridNetworkInterfaceClass))
    {
      interface = new ForwarderGridNetworkInterface(context, client, remoteInterface->getName());
      clientCommunication(context, interface, client);
    }
    else
    {
      context.warningCallback(connectedHostName, T("Unknown NetworkInterface - No communication protocol specified for this interface"));
      client->stopClient();
      continue;
    }
    
    /* Terminate the connection */
    context.informationCallback(connectedHostName, T("Disconnected"));
  }
}

void ManagerWorkUnit::serverCommunication(ExecutionContext& context, const ManagerNetworkInterfacePtr& interface, const NetworkClientPtr& client) const
{
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NetworkNotificationPtr notification;
    if (!client->receiveObject<NetworkNotification>(300000, notification) || !notification)
    {
      context.warningCallback(T("NetworkContext::run"), T("No notification received"));
      return;
    }

    if (notification.dynamicCast<CloseCommunicationNotification>())
      client->stopClient();

    notification->notifyNetwork(interface, client);
  }
}
  
void ManagerWorkUnit::clientCommunication(ExecutionContext& context, const GridNetworkInterfacePtr& interface, const NetworkClientPtr& client)
{
  String nodeName = interface->getName();
  if (nodeName == String::empty)
  {
    interface->getContext().warningCallback(client->getConnectedHostName(), T("Fail - Empty node name"));
    return;
  }

  /* Send new requests */
  std::vector<WorkUnitNetworkRequestPtr> waitingRequests;
  fileManager->getWaitingRequests(nodeName, waitingRequests);
  sendRequests(context, interface, client, waitingRequests);

  /* Get trace */
  ContainerPtr networkResponses = interface->getFinishedExecutionTraces();
  if (!networkResponses)
    return;
  
  size_t n = networkResponses->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    ExecutionTraceNetworkResponsePtr response = networkResponses->getElement(i).getObjectAndCast<ExecutionTraceNetworkResponse>();
    if (!response)
    {
      context.warningCallback(client->getConnectedHostName(), T("GetFinishedExecutionTraces - ExecutionTraceNetworkResponsePtr()"));
      continue;
    }
    WorkUnitNetworkRequestPtr request = fileManager->getRequest(response->getIdentifier());
    if (!request)
      continue;
    fileManager->archiveRequest(new NetworkArchive(request, response));
  }

  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();
}

void ManagerWorkUnit::sendRequests(ExecutionContext& context, GridNetworkInterfacePtr interface, const NetworkClientPtr& client, const std::vector<WorkUnitNetworkRequestPtr>& requests) const
{
  const size_t numRequests = requests.size();
  if (!numRequests)
    return;
  
  size_t numRequestsSent = 0;
  while (numRequestsSent < numRequests)
  {
    const size_t numThisTime = juce::jmin(numRequests - numRequestsSent, 200);
    /* Prepare data and send */
    ObjectVectorPtr v = objectVector(workUnitNetworkRequestClass, numThisTime);
    for (size_t i = 0; i < numThisTime; ++i)
      v->set(i, requests[numRequestsSent + i]);
    ContainerPtr results = interface->pushWorkUnits(v);
    /* Check acknowledgement */
    if (!results || results->getNumElements() != numThisTime)
    {
      context.warningCallback(client->getConnectedHostName(), T("PushWorkUnits - No acknowledgement received."));
      std::vector<WorkUnitNetworkRequestPtr> errorRequests(requests.begin() + numRequestsSent, requests.begin() + numRequestsSent + numThisTime);        
      fileManager->setAsWaitingRequests(errorRequests);
    }
    else
    {
      size_t n = results->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        String result = results->getElement(i).getString();
        if (result == T("Error"))
          fileManager->crachedRequest(requests[numRequestsSent + i]);
      }
    }
    
    numRequestsSent += numThisTime;
  }
}

/*
** GridWorkUnit
*/
Variable GridWorkUnit::run(ExecutionContext& context)
{
  if (gridEngine != T("SGE") && gridEngine != T("BOINC"))
    return false;
  
  /* Establishing a connection */
  NetworkClientPtr client = blockingNetworkClient(context, 3);
  if (!client->startClient(hostName, port))
  {
    context.warningCallback(T("NetworkContext::run"), T("Connection to ") + hostName.quoted() + (" fail !"));
    client->stopClient();
    return Variable();
  }
  context.informationCallback(T("NetworkContext::run"), T("Connected to ") + hostName + T(":") + String((int)port));
  
  NetworkInterfacePtr interface;
  if (gridEngine == T("SGE"))
    interface = new SgeGridNetworkInterface(context, client, gridName);
  else if (gridEngine == T("BOINC"))
    interface = new BoincGridNetworkInterface(context, client, gridName);
  else
  {
    jassertfalse;
    return Variable();
  }
  
  /* Slave mode - Execute received commands */
  client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(interface));
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NetworkNotificationPtr notification;
    if (!client->receiveObject<NetworkNotification>(300000, notification) || !notification)
    {
      context.warningCallback(T("NetworkContext::run"), T("No notification received"));
      return false;
    }
    notification->notifyNetwork(interface, client);
  }
  return true;
}

/*
** DumbWorkUnit
*/
Variable DumbWorkUnit::run(ExecutionContext& context)
{
  for (size_t i = 0; i < 10; ++i)
  {
    context.progressCallback(new ProgressionState(i+1, 10, T("DumbWorkUnit")));
    juce::Thread::sleep(1000);
  }
  return Variable();
}

/*
** GetTraceWorkUnit
*/
Variable GetTraceWorkUnit::run(ExecutionContext& context)
{
  NetworkClientPtr client = blockingNetworkClient(context);
  if (!client->startClient(hostName, port))
  {
    context.errorCallback(T("GetTraceWorkUnit::run"), T("Not connected !"));
    return false;
  }
  context.informationCallback(hostName, T("Connected !"));
  
  ManagerNetworkInterfacePtr interface = forwarderManagerNetworkInterface(context, client, T("Client"));
  
  client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(interface));
  if (!interface->isFinished(workUnitIdentifier))
  {
    context.informationCallback(T("GetTraceWorkUnit::run"), T("The Work Unit (") + workUnitIdentifier + T(") is not finished !"));
    return true;
  }
  
  ExecutionTraceNetworkResponsePtr res = interface->getExecutionTrace(workUnitIdentifier);
  if (!res)
  {
    context.errorCallback(T("GetTraceWorkUnit::run"), T("No ExecutionTraceNetworkResponse received !"));
    return false;
  }

  ExecutionTracePtr trace = res->getExecutionTrace(context);
  if (!trace)
  {
    context.errorCallback(T("GetTraceWorkUnit::run"), T("No ExecutionTrace available ! Maybe due to a crash on the grid."));
    return false;
  }
  
  File dst = context.getFile(workUnitIdentifier + T(".trace"));
  trace->saveToFile(context, dst);
  context.informationCallback(T("The trace has been saved to ") + dst.getFullPathName().quoted());
  
  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();

  return true;
}