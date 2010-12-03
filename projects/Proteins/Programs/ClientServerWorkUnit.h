/*******************************************************************************
 ! WARNING !   Test Zone - Do not enter !
 *******************************************************************************/

#include "NetworkClient.h"
#include "NetworkServer.h"
#include "Command.h"

namespace lbcpp
{
  
class DumbWorkUnit : public WorkUnit
{
public:
  DumbWorkUnit() {}
  
  virtual bool run(ExecutionContext& context)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      context.progressCallback(i+1, 10, T("DumbWorkUnit"));
      juce::Thread::sleep(1000);
    }
    return true;
  }
};

class ServerWorkUnit : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    NetworkServerPtr server = new NetworkServer(context);
    server->startServer(1664);
    
    NetworkClientPtr client = server->acceptClient(true);
    std::cout << "* New client: " << client->getConnectedHostName() << std::endl;
    
    BufferedNetworkCallbackPtr callback = new BufferedNetworkCallback();
    client->appendCallback(callback);
    
    client->sendVariable(informationContextCommand(T("PING PONG!")));
    
    client->sendVariable(echoCommand(T("PING PONG!")));
    Variable v = callback->receiveVariable(true);
    v.getObjectAndCast<Command>(context)->runCommand(context, client);
    
    client->sendVariable(systemStatCommand());
    v = callback->receiveVariable(true);
    SystemStatsPtr stats = v.getObjectAndCast<SystemStats>(context);
    jassert(stats);
    context.informationCallback(client->getConnectedHostName(), stats->toString());
    
    client->sendVariable(workUnitCommand(new DumbWorkUnit()));
    client->sendVariable(workUnitCommand(new DumbWorkUnit()));
    client->sendVariable(workUnitCommand(new DumbWorkUnit()));
    client->sendVariable(workUnitCommand(new DumbWorkUnit()));
    
    //client->sendVariable(Variable());
    
    client->stopClient();
    server->stopServer();
    
    return true;
  }
};

class ClientWorkUnit : public WorkUnit
{
public:
  ClientWorkUnit() : hostname(T("192.168.1.3")) {}
  
  virtual bool run(ExecutionContext& context)
  {
    NetworkClientPtr client = blockingNetworkClient(context, 3);
    BufferedNetworkCallbackPtr callback = new BufferedNetworkCallback();
    client->appendCallback(callback);
    
    if (!client->startClient(hostname, 1664))
    {
      context.informationCallback(T("ClientWorkUnit::Networking"), T("Connection fail !"));
      client->stopClient();
      return false;
    }
    
    context.informationCallback(T("ClientWorkUnit::Networking"), T("Connected to ") + client->getConnectedHostName());
    
    while (true)
    {
      Variable v = callback->receiveVariable(true);
      if (v.isNil())
        break;
      
      CommandPtr cmd = v.getObjectAndCast<Command>(context);
      jassert(cmd);
      
      if (cmd->callOnCurrentThread())
        cmd->runCommand(context, client);
      else
        units.push_back(cmd);
    }
    client->stopClient();
    
    while (units.size())
    {
      context.informationCallback(T("ClientWorkUnit::run"), T("Job remaining: ") + String((int)units.size()));
      context.pushWorkUnit(units.front());
      units.pop_front();
    }
    
    return true;
  }
  
protected:
  friend class ClientWorkUnitClass;
  String hostname;
  
  std::deque<WorkUnitPtr> units;
};
  
}; /* namespace lbcpp */
