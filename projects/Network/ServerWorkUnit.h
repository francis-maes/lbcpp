#include "NetworkCommand.h"

namespace lbcpp
{

class ServerWorkUnit : public WorkUnit
{
public:
  ServerWorkUnit(File workUnitDirectory = File::getCurrentWorkingDirectory())
    : workUnitDirectory(workUnitDirectory), hostname(T("monster24.montefiore.ulg.ac.be")), serverName(T("unnamed")) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    ServerNetworkContextPtr networkContext = new SgeServerNetworkContext(serverName, hostname, 1664, workUnitDirectory);    
    networkContext->run(context);
    return Variable();
  }

protected:
  friend class ServerWorkUnitClass;

  File workUnitDirectory;
  String hostname;
  String serverName;
};

class GridWorkUnit : public WorkUnit
{
public:
  GridWorkUnit(const String& gridName, const String& gridEngine, const String& hostName, size_t port)
    : gridName(gridName), gridEngine(gridEngine), hostName(hostName), port(port) {}
  GridWorkUnit() : port(1664) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    NodeNetworkInterfacePtr interface;
    if (gridEngine == T("SGE"))
      interface = new SgeNodeNetworkInterface();
    else
    {
      jassertfalse;
      return Variable();
    }
    interface->setContext(context);

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
  
protected:
  String gridName;
  String gridEngine;
  String hostName;
  size_t port;
};

};