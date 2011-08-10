/*-----------------------------------------.---------------------------------.
| Filename: NetworkWorkUnit.h              | Network Work Unit               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_WORK_UNIT_H_
# define LBCPP_NETWORK_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Network/NetworkInterface.h>
# include "../Manager/ManagerNetworkInterface.h"
# include "../Grid/GridNetworkInterface.h"
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkServer.h>
# include <lbcpp/Network/NetworkMessage.h>

namespace lbcpp
{

class ManagerWorkUnit : public WorkUnit
{
public:
  ManagerWorkUnit() : port(1664) {}
  
  virtual Variable run(ExecutionContext& context);

protected:
  friend class ManagerWorkUnitClass;

  size_t port;
  NetworkProjectFileManagerPtr fileManager;

  void serverCommunication(ExecutionContext& context, const ManagerNetworkInterfacePtr& interface, const NetworkClientPtr& client) const;
  void clientCommunication(ExecutionContext& context, const GridNetworkInterfacePtr& interface, const NetworkClientPtr& client);
  
private:
  void sendRequests(ExecutionContext& context, GridNetworkInterfacePtr interface, const NetworkClientPtr& client, const std::vector<WorkUnitNetworkRequestPtr>& requests) const; 
};

class GridWorkUnit : public WorkUnit // => ClientWorkUnit
{
public:
  GridWorkUnit(const String& gridName, const String& gridEngine, const String& hostName, size_t port)
    : gridName(gridName), gridEngine(gridEngine), hostName(hostName), port(port) {}
  GridWorkUnit() : port(1664) {}

  virtual Variable run(ExecutionContext& context);

protected:
  friend class GridWorkUnitClass;

  String gridName;    // clientHostName
  String gridEngine;  // remplacer par NetworkInterfacePtr clientInterface   // -i SGE
  String hostName;    // serverHostName
  size_t port;        // serverPort
};

class DumbWorkUnit : public WorkUnit
{
public:
  DumbWorkUnit() {}

  virtual Variable run(ExecutionContext& context);
};

class GetTraceWorkUnit : public WorkUnit
{
public:
  GetTraceWorkUnit()
    : hostName(T("monster24.montefiore.ulg.ac.be")), port(1664) {}

  virtual Variable run(ExecutionContext& context);

protected:
  friend class GetTraceWorkUnitClass;
  
  String workUnitIdentifier;
  String hostName;
  size_t port;
};

/** xxxServerWorkUnit **/
class XxxLocalNetworkClientThread : public Thread
{
public:
  XxxLocalNetworkClientThread(XxxNetworkClientPtr client, WorkUnitNetworkMessagePtr message, const String& uniqueIdentifier)
    : Thread(T("LocalNetworkClientThread ") + uniqueIdentifier)
    , client(client), message(message), uniqueIdentifier(uniqueIdentifier)
    {}

  virtual void run()
  {
    client->sendVariable(new WorkUnitAcknowledgementNetworkMessage(message->getSourceIdentifier(), uniqueIdentifier));
    Variable res = client->getContext().run(message->getWorkUnit(client->getContext()));
    client->sendVariable(new WorkUnitResultNetworkMessage(client->getContext(), uniqueIdentifier, res));
  }

protected:
  XxxNetworkClientPtr client;
  WorkUnitNetworkMessagePtr message;
  String uniqueIdentifier;
};

typedef ReferenceCountedObjectPtr<XxxLocalNetworkClientThread> XxxLocalNetworkClientThreadPtr;

class XxxLocalNetworkClient : public XxxNetworkClient
{
public:
  XxxLocalNetworkClient(ExecutionContext& context)
    : XxxNetworkClient(context), lastWorkUnitId(0) {}

  virtual void variableReceived(const Variable& variable)
  {
    size_t workUnitId = 0;
    {
      ScopedLock _(lock);
      workUnitId = ++lastWorkUnitId;
    }

    if (!variable.isObject())
    {
      context.warningCallback(T("xxxLocalNetworkClient::variableReceived")
                              , T("The message is not an Object ! The message is ") + variable.toString().quoted());
      return;
    }

    const ObjectPtr obj = variable.getObject();
    if (!obj)
    {
      context.warningCallback(T("xxxLocalNetworkClient::variableReceived"), T("NULL Object"));
      return;
    }

    const ClassPtr objClass = obj->getClass();
    if (objClass == workUnitNetworkMessageClass)
    {
      WorkUnitNetworkMessagePtr message = obj.staticCast<WorkUnitNetworkMessage>();
      XxxLocalNetworkClientThreadPtr thread = new XxxLocalNetworkClientThread(refCountedPointerFromThis(this), message, String((int)workUnitId));
      thread->startThread();
    }
    else
    {
      context.warningCallback(T("xxxLocalNetworkClient::variableReceived")
                              , T("Unknwon object of type: ") + objClass->toString());
    }
  }

  virtual void connectionLost() {}

  lbcpp_UseDebuggingNewOperator

protected:
  CriticalSection lock;
  volatile size_t lastWorkUnitId;
};

class XxxLocalNetworkServer : public XxxNetworkServer
{
public:
  XxxLocalNetworkServer(ExecutionContext& context)
    : XxxNetworkServer(context) {}

  virtual XxxNetworkClient* createNetworkClient()
    {return new XxxLocalNetworkClient(context);}

  lbcpp_UseDebuggingNewOperator

};

class XxxServerWorkUnit : public WorkUnit
{
public:
  XxxServerWorkUnit(const String& serverType = T("local"), size_t port = 1664)
    : serverType(serverType), port(port) {}

  Variable run(ExecutionContext& context)
  {
    if (serverType == T("local"))
    {
      XxxNetworkServerPtr server = new XxxLocalNetworkServer(context);
      return server->startServer(port);
    }

    context.errorCallback(T("XxxServerWorkUnit:run"), T("Unknown server: ") + serverType.quoted());
    return false;
  }

protected:
  friend class XxxServerWorkUnitClass;

  String serverType;
  size_t port;
};

class XxxClientWorkUnit : public WorkUnit
{
public:
  XxxClientWorkUnit(String hostName = T("localhost"), size_t hostPort = 1664)
    : hostName(hostName), hostPort(hostPort) {}

  Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, hostName, hostPort);
    
    CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("Fuck them all"));
    for (size_t i = 0; i < 10; ++i)
      workUnits->addWorkUnit(new DumbWorkUnit());
    Variable result = remoteContext->run(workUnits);

    context.informationCallback(result.toString());

    return true;
  }

protected:
  friend class XxxClientWorkUnitClass;

  String hostName;
  size_t hostPort;
};

}; /* namespace */

#endif // !LBCPP_NETWORK_WORK_UNIT_H_
