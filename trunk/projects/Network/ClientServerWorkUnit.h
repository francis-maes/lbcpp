/*******************************************************************************
 ! WARNING !   Test Zone - Do not enter !
 *******************************************************************************/

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
    
    NetworkClientPtr client = server->acceptClient(true); // TODO timeout
    std::cout << "* New client: " << client->getConnectedHostName() << std::endl;
    
    BufferedNetworkCallbackPtr callback = new BufferedNetworkCallback();
    client->appendCallback(callback);
    
    client->sendVariable(informationContextCommand(T("PING PONG!")));
  
    client->sendVariable(echoCommand(T("PING PONG!")));
    Variable v = callback->receiveVariable(true);
    v.getObjectAndCast<Command>(context)->runCommand(context, client);
    
    client->sendVariable(systemStatCommand());
    v = callback->receiveVariable(true); // TODO timeout
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

/*******************************************************************************
              ! WARNING !         >> Client Side <<
*******************************************************************************/
  
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
      {
        context.pushWorkUnit(cmd);
        client->sendVariable(informationContextCommand(T("Work unit ") + cmd->getName().quoted() + T(" correctly received")));
      }
    }
    client->stopClient();
    
    return true;
  }
  
protected:
  friend class ClientWorkUnitClass;
  String hostname;
  
  std::deque<WorkUnitPtr> units;
};

/*******************************************************************************
                ! WARNING !         >>  <<
*******************************************************************************/
  
}; /* namespace lbcpp */
