/*-----------------------------------------.---------------------------------.
| Filename: NetworkWorkUnit.cpp            | Network Work Unit               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "../Node/NodeNetworkNotification.h"
#include "NetworkWorkUnit.h"

#include <lbcpp/Network/NetworkServer.h>

using namespace lbcpp;

/*
** ManagerWorkUnit
*/

Variable ManagerWorkUnit::run(ExecutionContext& context)
{
  NetworkServerPtr server = new NetworkServer(context);
  if (!server->startServer(port))
  {
    context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port ") + String((int)port));
    return false;
  }
  ManagerNodeNetworkInterfacePtr managerInterface = new ManagerNodeNetworkInterface(context);
  
  while (true)
  {
    /* Accept client */
    NetworkClientPtr client = server->acceptClient(INT_MAX);
    if (!client)
      continue;
    context.informationCallback(client->getConnectedHostName(), T("connected"));
    
    /* Which kind of connection ? */
    String className;
    if (!client->receiveString(10000, className) || className == String::empty)
    {
      context.warningCallback(client->getConnectedHostName(), T("Fail - Client type (1)"));
      client->stopClient();
      continue;
    }
    ClassPtr type = typeManager().getType(context, className);
    
    /* Strat communication (depending of the type) */
    NetworkInterfacePtr interface;
    if (type->inheritsFrom(clientNodeNetworkInterfaceClass))
    {
      interface = managerInterface;
      interface->setNetworkClient(client);
      
      serverCommunication(interface);
    }
    else if (type->inheritsFrom(nodeNetworkInterfaceClass))
    {
      interface = new ClientNodeNetworkInterface(context, client);
      clientCommunication(interface, managerInterface);
    }
    else
    {
      context.warningCallback(client->getConnectedHostName(), T("Fail - Client type (2)"));
      client->stopClient();
      continue;
    }
    
    /* Terminate the connection */
    interface->closeCommunication(context);
    context.informationCallback(client->getConnectedHostName(), T("disconnected"));
  }
}

void ManagerWorkUnit::serverCommunication(NodeNetworkInterfacePtr interface) const
{
  NetworkClientPtr client = interface->getNetworkClient();
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NotificationPtr notification;
    if (!client->receiveObject<Notification>(10000, notification) || !notification)
    {
      interface->getContext().warningCallback(T("NetworkContext::run"), T("No notification received"));
      return;
    }
    notification->notify(interface);
  }
}
  
void ManagerWorkUnit::clientCommunication(NodeNetworkInterfacePtr interface, ManagerNodeNetworkInterfacePtr manager)
{
  String nodeName = interface->getNodeName(interface->getContext());
  if (nodeName == String::empty)
  {
    interface->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("Fail - Empty node name"));
    return;
  }

  /* Send new requests */
  std::vector<NetworkRequestPtr> waitingRequests;
  manager->getRequestsWithStatus(nodeName, WorkUnitInformation::waitingOnManager, waitingRequests);
  for (size_t i = 0; i < waitingRequests.size(); ++i)
  {
    WorkUnitInformationPtr info = interface->pushWorkUnit(interface->getContext(), waitingRequests[i]);
    updateStatus(interface, manager, waitingRequests[i], info);
  }

  /* Update status */
  ObjectVectorPtr modifiedRequests = interface->getModifiedStatusSinceLastConnection(interface->getContext());
  if (!modifiedRequests)
    return;
  
  size_t n = modifiedRequests->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    WorkUnitInformationPtr info = modifiedRequests->getAndCast<WorkUnitInformation>(i);
    if (!info)
    {
      manager->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("Empty modified request"));
      continue;
    }
    updateStatus(interface, manager, manager->getRequest(info), info);
  }
}

void ManagerWorkUnit::updateStatus(NodeNetworkInterfacePtr interface, ManagerNodeNetworkInterfacePtr manager, NetworkRequestPtr request, WorkUnitInformationPtr info)
{
  if (!request || !info)
  {
    manager->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("Request or WorkUntInformation does not exists in ManagerWorkUnit::updateStatus"));
    return;
  }

  WorkUnitInformation::Status oldStatus = request->getWorkUnitInformation()->getStatus();
  WorkUnitInformation::Status newStatus = info->getStatus();

  if (newStatus == oldStatus)
    return;

  request->getWorkUnitInformation()->setStatus(newStatus);

  if (newStatus == WorkUnitInformation::iDontHaveThisWorkUnit)
  {
    manager->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("WorkUnitInformation::iDontHaveThisWorkUnit - Re-send work unit"));
    interface->pushWorkUnit(interface->getContext(), request);
    request->getWorkUnitInformation()->setStatus(WorkUnitInformation::waitingOnServer);
    return;
  }
  
  if (newStatus == WorkUnitInformation::workUnitError)
  {
    manager->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("WorkUnitInformation::workUnitError - Node have trouble with this work unit: ") + info->getProjectName() + T("/") + info->getIdentifier());
    NetworkResponsePtr response = interface->getExecutionTrace(interface->getContext(), info); // to remove server files
    manager->setRequestAsFail(manager->getContext(), request, response);
    return;
  }
  
  if (newStatus == WorkUnitInformation::finished)
  {
    NetworkResponsePtr response = interface->getExecutionTrace(interface->getContext(), info);
    manager->archiveRequest(manager->getContext(), request, response);
    return;
  }

  manager->saveRequest(manager->getContext(), request);
}

/*
** GridWorkUnit
*/

Variable GridWorkUnit::run(ExecutionContext& context)
{
  NodeNetworkInterfacePtr interface;
  if (gridEngine == T("SGE"))
    interface = new SgeNodeNetworkInterface(context, gridName);
  else
  {
    jassertfalse;
    return Variable();
  }
  
  /* Establishing a connection */
  NetworkClientPtr client = blockingNetworkClient(context, 3);
  if (!client->startClient(hostName, port))
  {
    context.warningCallback(T("NetworkContext::run"), T("Connection to ") + hostName.quoted() + ("fail !"));
    client->stopClient();
    return Variable();
  }
  context.informationCallback(T("NetworkContext::run"), T("Connected to ") + hostName + T(":") + String((int)port));
  interface->setNetworkClient(client);
  
  /* Slave mode - Execute received commands */
  interface->sendInterfaceClass();
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NotificationPtr notification;
    if (!client->receiveObject<Notification>(10000, notification) || !notification)
    {
      context.warningCallback(T("NetworkContext::run"), T("No notification received"));
      return false;
    }
    notification->notify(interface);
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
** ClientWorkUnit
*/


Variable ClientWorkUnit::run(ExecutionContext& context)
{
  NetworkClientPtr client = blockingNetworkClient(context);
  
  if (!client->startClient(hostName, port))
  {
    context.errorCallback(T("ClientWorkUnit::run"), T("Not connected !"));
    return Variable();
  }
  
  NodeNetworkInterfacePtr interface = new ClientNodeNetworkInterface(context, client, clientName);
  interface->sendInterfaceClass();
  
  /* Submit jobs */
  for (size_t i = 0; i < 1; ++i)
  {
    NetworkRequestPtr request = new NetworkRequest(context, new WorkUnitInformation(T("testProject"), clientName, T("LocalGridNode")), new DumbWorkUnit());
    WorkUnitInformationPtr res = interface->pushWorkUnit(context, request);
    interface->getWorkUnitStatus(context, res);
  }
  
  interface->closeCommunication(context);
  
  return Variable();
}
