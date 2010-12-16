#include <lbcpp/lbcpp.h>
#include "NetworkCommand.h"

namespace lbcpp
{
  
class ClientManager : public Object
{
public:
  ClientManager(const String& clientName = String::empty) : clientName(clientName), nextJobIdentifier(0) {}
  
  size_t appendWorkUnit(WorkUnitPtr workunit)
  {
    ScopedLock _(lock);
    waiting.push_back(workunit);
    return getNextJobIdentifier();
  }
  
  String getClientName() const
    {return clientName;}
  
protected:
  String clientName;
  size_t nextJobIdentifier;
  std::vector<WorkUnitPtr> waiting;
  std::vector<WorkUnitPtr> inProgress;
  std::vector<WorkUnitPtr> finised;
  
  CriticalSection lock;
  
  size_t getNextJobIdentifier()
    {return nextJobIdentifier++;}
};

typedef ReferenceCountedObjectPtr<ClientManager> ClientManagerPtr;

class ManagerServer : public WorkUnit
{
public:
  ManagerServer() : port(1664) {}
  
  bool run(ExecutionContext& context)
  {
    NetworkServerPtr server = new NetworkServer(context);
    if (!server->startServer(port))
    {
      context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port ") + String(port));
      return false;
    }

    while (true)
    {
      NetworkClientPtr client = server->acceptClient(INT_MAX);
      if (!client)
        continue;
      String hostname = client->getConnectedHostName();
      context.informationCallback(hostname, T("connected"));

      /* Type of client */
      client->sendVariable(new GetConnectionTypeNetworkCommand());
      String clientType;
      if (!client->receiveString(10000, clientType))
      {
        context.warningCallback(hostname, T("Fail - Client type"));
        closeConnection(context, client);
        continue;
      }
      context.informationCallback(hostname, T("Connection type: ") + clientType);

      /* Blablating */
      if (clientType == T("Server"))
        serverCommunication(context, client);
      if (clientType == T("Client"))
        clientCommunication(context, client);

      closeConnection(context, client);
    }
  }

protected:
  int port;
  std::vector<WorkUnitPtr> waitingJobs;
  std::vector<ClientManagerPtr> clientManagers;
  
  void closeConnection(ExecutionContext& context, NetworkClientPtr client)
  {
    /* Terminate the connection */
    if (client->isConnected())
    {
      client->sendVariable(new CloseConnectionNetworkCommand());
      client->stopClient();
    }
    context.informationCallback(client->getConnectedHostName(), T("disconnected"));
  }

  bool clientCommunication(ExecutionContext& context, NetworkClientPtr client)
  {
    std::cout << "Communicate - ClientContextInformation" << std::endl;
    String hostname = client->getConnectedHostName();
    /* Get client identifier */
    client->sendVariable(new GetIdentityNetworkCommand());
    String identity;
    if (!client->receiveString(10000, identity))
    {
      context.warningCallback(hostname, T("Fail - Identity"));
      return false;
    }
    context.informationCallback(hostname, T("Name: ") + identity);

    while (true)
    {
      /* Ask and Wait for a job */
      client->sendVariable(new GetWorkUnitNetworkCommand());
      WorkUnitPtr workUnit;
      if (!client->receiveObject<WorkUnit>(10000, workUnit))
      {
        context.warningCallback(hostname, T("Fail - GetWorkUnit"));
        return false;
      }
      
      if (!workUnit)
        break;

      /* Generate an identifier */
      ClientManagerPtr manager;
      for (size_t i = 0; i < clientManagers.size(); ++i)
        if (clientManagers[i]->getClientName() == identity)
          manager = clientManagers[i];
      if (!manager)
        manager = new ClientManager(identity);
      size_t identifier = manager->appendWorkUnit(workUnit);
      /* Return the identifier */
      client->sendVariable(Variable((int)identifier, positiveIntegerType));
      
      context.informationCallback(hostname, T("WorkUnit ID: ") + String((int)identifier));
    }

    return true;
  }
  
  bool serverCommunication(ExecutionContext& context, NetworkClientPtr client)
  {
    std::cout << "Communicate - ServerContextInformation" << std::endl;
#if 0
    /* Get job status */
    
    /* Send job */
    if (!waitingJobs.size())
      return true;
    
    SystemResourcePtr resource;
    for (size_t i = 0; i < waitingJobs.size(); ++i)
    {
      /* Get availability ressouces */
      if (!resource)
      {
        client->sendVariable(new GetResourceCommand());
        resource = receiveVariable<SystemResource>(context, client, true);
        if (!resource)
          return false;
      }
      /* Check if ressouces compatible */
      if (resource->isSufficient(waitingJobs[i]))
      {
        client->sendVariable(waitingJobs[i]);
        waitingJobs[i] = WorkUnitPtr();
        // TODO add job to manager
      }
      // TODO remove empty job from list
    }
#endif
    return false;
  }
};

};
