/*-----------------------------------------.---------------------------------.
| Filename: Command.h                      | Network Command                 |
| Author  : Julien Becker                  |                                 |
| Started : 02/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_COMMAND_H_
# define LBCPP_NETWORK_COMMAND_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

extern ClassPtr clientNetworkContextClass;
extern ClassPtr serverNetworkContextClass;

class NetworkContext;
typedef ReferenceCountedObjectPtr<NetworkContext> NetworkContextPtr;
  
class NetworkCommand : public Object
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network) = 0;
};

typedef ReferenceCountedObjectPtr<NetworkCommand> NetworkCommandPtr;

class ClientNetworkContext;
typedef ReferenceCountedObjectPtr<ClientNetworkContext> ClientNetworkContextPtr;

class ServerNetworkContext;
typedef ReferenceCountedObjectPtr<ServerNetworkContext> ServerNetworkContextPtr;

class NetworkContext : public WorkUnit
{
public:
  NetworkContext(const String& identity, const String& hostname, int port)
  : identity(identity), hostname(hostname), port(port) {}
  NetworkContext() {}
  
  virtual bool run(ExecutionContext& context)
  {
    /* Establishing a connection */
    client = blockingNetworkClient(context, 3);

    if (!client->startClient(hostname, port))
    {
      context.warningCallback(T("NetworkContext::run"), T("Connection fail !"));
      client->stopClient();
      return false;
    }
    context.informationCallback(T("NetworkContext::run"), T("Connected to ") + hostname + T(":") + String(port));
    
    /* Slave mode - Execute received commands */
    while (client->isConnected())
    {
      NetworkCommandPtr command;
      if (!client->receiveObject<NetworkCommand>(10000, command) || !command)
      {
        context.warningCallback(T("NetworkContext::run"), T("No command received"));
        return false;
      }
      
      command->runCommand(context, NetworkContextPtr(this));
    }
    return true;
  }
  
  NetworkClientPtr getNetworkClient() const
    {return client;}
  
  void stopClient()
    {client->stopClient();}
  
  String getIdentity() const
    {return identity;}

protected:
  String identity;
  String hostname;
  int port;
  
  NetworkClientPtr client;
};

class GetIdentityNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
    {return network->getNetworkClient()->sendVariable(network->getIdentity());}
};

class CloseConnectionNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    network->stopClient();
    return true;
  }
};

class GetConnectionTypeNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    if (network->getClass()->inheritsFrom(clientNetworkContextClass))
      return network->getNetworkClient()->sendVariable(String(T("Client")));
    if (network->getClass()->inheritsFrom(serverNetworkContextClass))
      return network->getNetworkClient()->sendVariable(String(T("Server")));
    context.errorCallback(T("GetConnectionTypeNetworkCommand::runCommand"), T("Unknown NetworkContext"));
    return false;
  }
};

class ClientNetworkContext : public NetworkContext
{
public:
  ClientNetworkContext(const String& identity, const String& hostname, int port)
  : NetworkContext(identity, hostname, port) {}
  ClientNetworkContext() {}
  
  void pushWorkUnit(WorkUnitPtr workUnit)
  {
    ScopedLock _(lock);
    workUnits.push_back(workUnit);
  }
  
  WorkUnitPtr popWorkUnit()
  {
    ScopedLock _(lock);
    WorkUnitPtr res = workUnits.front();
    workUnits.pop_front();
    return res;
  }

protected:
  std::deque<WorkUnitPtr> workUnits;
  CriticalSection lock;
};

class GetWorkUnitNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ClientNetworkContextPtr clientNetwork = network.staticCast<ClientNetworkContext>();
    if (!clientNetwork)
    {
      context.errorCallback(T("GetWorkUnitNetworkCommand::runCommand"), T("Must be execute in a ClientNetworkContext only"));
      return false;
    }
    WorkUnitPtr workUnit = clientNetwork->popWorkUnit();
    clientNetwork->getNetworkClient()->sendVariable(workUnit);
    
    if (!workUnit)
      return true;
    
    // FIXME
    return true;
  }
};

class ServerNetworkContext : public NetworkContext
{
public:
  String getWorkUnitStatus(juce::int64 workUnitId)
  {
    return T("IDontHaveThisWorkUnit");
  }
  
  void pushWorkUnit(juce::int64 workUnitId, WorkUnitPtr workUnit)
  {
    std::cout << "Server - Work unit received: " << workUnitId << std::endl;
  }
};

class GetWorkUnitStatusNetworkCommand : public NetworkCommand
{
public:
  GetWorkUnitStatusNetworkCommand(juce::int64 workUnitId = 0) : workUnitId(workUnitId) {}
  
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }
    
    network->getNetworkClient()->sendVariable(serverNetwork->getWorkUnitStatus(workUnitId));
    return true;
  }

protected:
  friend class GetWorkUnitStatusNetworkCommandClass;
  
  juce::int64 workUnitId;
};

class SystemResource : public Object
{
public:
  bool isSufficient(WorkUnitPtr workUnit) const
    {return true;} // FIXME
};

typedef ReferenceCountedObjectPtr<SystemResource> SystemResourcePtr;

class GetSystemResourceNetworkCommand : public NetworkCommand
{
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }

    // FIXME
    serverNetwork->getNetworkClient()->sendVariable(SystemResourcePtr(new SystemResource()));
    return true;
  }
};

class PushWorkUnitNetworkCommand : public NetworkCommand
{
public:
  PushWorkUnitNetworkCommand(juce::int64 workUnitId, WorkUnitPtr workUnit)
  : workUnitId(workUnitId), workUnit(workUnit) {}
  PushWorkUnitNetworkCommand() {}
  
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }
   
    serverNetwork->pushWorkUnit(workUnitId, workUnit);
    return true;
  }
  
protected:
  friend class PushWorkUnitNetworkCommandClass;
  
  juce::int64 workUnitId;
  WorkUnitPtr workUnit;
};

}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_COMMAND_H_
