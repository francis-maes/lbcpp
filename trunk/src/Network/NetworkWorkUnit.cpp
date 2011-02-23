/*-----------------------------------------.---------------------------------.
| Filename: NetworkWorkUnit.cpp            | Network Work Unit               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NetworkNotification.h"
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
  
void ManagerWorkUnit::clientCommunication(NodeNetworkInterfacePtr interface, ManagerNodeNetworkInterfacePtr manager) const
{
  String nodeName = interface->getNodeName(interface->getContext());
  if (nodeName == String::empty)
  {
    interface->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("Fail - Empty node name"));
    return;
  }
  
  /* Update status */
  std::vector<WorkUnitNetworkRequestPtr> requests;
  manager->getUnfinishedRequestsSentTo(nodeName, requests);
  for (size_t i = 0; i < requests.size(); ++i)
  {
    int status = interface->getWorkUnitStatus(interface->getContext(), requests[i]->getNetworkRequest());
    int oldStatus = requests[i]->getStatus();

    if (status == NetworkRequest::iDontHaveThisWorkUnit) // implicitly send new request
    {
      interface->pushWorkUnit(interface->getContext(), requests[i]);
      continue;
    }

    if (status == NetworkRequest::finished && status != oldStatus) // transition to finised status
    {
      ExecutionTracePtr trace = interface->getExecutionTrace(interface->getContext(), requests[i]->getNetworkRequest());
      manager->archiveTrace(manager->getContext(), requests[i], trace);
      continue;
    }

    requests[i]->setStatus(status);
    if (oldStatus != status)
    {
      File f = manager->getContext().getFile(T("Requests/") + requests[i]->getIdentifier() + T(".request"));
      requests[i]->saveToFile(manager->getContext(), f);
    }
  }
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
    WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, new DumbWorkUnit(), T("testProject"), clientName, T("LocalGridNode"));
    NetworkRequestPtr res = interface->pushWorkUnit(context, request);
    interface->getWorkUnitStatus(context, res);
  }
  
  interface->closeCommunication(context);
  
  return Variable();
}
