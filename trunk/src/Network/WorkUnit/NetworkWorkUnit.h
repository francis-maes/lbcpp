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
  XxxLocalNetworkClientThread(const XxxGridNetworkClientPtr& client, const WorkUnitPtr& workUnit, const String& uniqueIdentifier)
    : Thread(T("LocalNetworkClientThread ") + uniqueIdentifier)
    , client(client), workUnit(workUnit), uniqueIdentifier(uniqueIdentifier)
    {}

  virtual void run()
  {
    Variable res = client->getContext().run(workUnit);
    client->sendWorkUnitResult(uniqueIdentifier, res);
  }

protected:
  XxxGridNetworkClientPtr client;
  WorkUnitPtr workUnit;
  String uniqueIdentifier;
};

typedef ReferenceCountedObjectPtr<XxxLocalNetworkClientThread> XxxLocalNetworkClientThreadPtr;

class XxxLocalGridNetworkClient : public XxxGridNetworkClient, public XxxGridNetworkClientCallback
{
public:
  XxxLocalGridNetworkClient(ExecutionContext& context)
    : XxxGridNetworkClient(context, this), lastWorkUnitId(0) {}

  virtual void workUnitRequestReceived(size_t sourceIdentifier, const XmlElementPtr& xmlWorkUnit)
  {
    size_t uniqueIdentifier = 0;
    {
      ScopedLock _(lock);
      uniqueIdentifier = ++lastWorkUnitId;
    }
    sendWorkUnitAcknowledgement(sourceIdentifier, String((int)uniqueIdentifier));
    XxxLocalNetworkClientThreadPtr thread = new XxxLocalNetworkClientThread(refCountedPointerFromThis(this)
                                                                            , xmlWorkUnit->createObjectAndCast<WorkUnit>(context)
                                                                            , String((int)uniqueIdentifier));
    thread->startThread();
  }

  virtual bool sendWorkUnitAcknowledgement(size_t sourceIdentifier, const String& uniqueIdentifier)
  {
    return sendVariable(new WorkUnitAcknowledgementNetworkMessage(sourceIdentifier, uniqueIdentifier));
  }

  virtual bool sendWorkUnitResult(const String& uniqueIdentifier, const Variable& result)
  {
    return sendVariable(new WorkUnitResultNetworkMessage(context, uniqueIdentifier, result));
  }

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
    {return new XxxLocalGridNetworkClient(context);}

  lbcpp_UseDebuggingNewOperator
};

class NetworkManager : public Object
{
public:
  NetworkManager(ExecutionContext& context)
    : context(context) {}

  void addRequest(const ObjectPtr& request) {}
  void archiveRequest(const ObjectPtr& archive) {}
  void crachedRequest(const ObjectPtr& request) {}
  ObjectPtr getRequest(const String& identifier) const {return ObjectPtr();}
  void getWaitingRequests(const String& nodeName, std::vector<ObjectPtr>& results) {}
  void setAsWaitingRequests(const std::vector<ObjectPtr>& networkRequests) {}

protected:
  ExecutionContext& context;
  CriticalSection lock;
};

typedef ReferenceCountedObjectPtr<NetworkManager> NetworkManagerPtr;

class XxxManagerServerNetworkClient : public XxxNetworkClient
{
public:
  XxxManagerServerNetworkClient(ExecutionContext& context, const NetworkManagerPtr& manager)
    : XxxNetworkClient(context), manager(manager) {jassert(manager);}

  virtual void variableReceived(const Variable& variable)
    {jassertfalse;}

protected:
  NetworkManagerPtr manager;
};

class XxxManagerNetworkServer : public XxxNetworkServer
{
public:
  XxxManagerNetworkServer(ExecutionContext& context, NetworkManagerPtr manager)
    : XxxNetworkServer(context), manager(manager) {}

  virtual XxxNetworkClient* createNetworkClient()
    {return new XxxManagerServerNetworkClient(context, manager);}

  lbcpp_UseDebuggingNewOperator

protected:
  NetworkManagerPtr manager;
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
    else if (serverType == T("manager"))
    {
      NetworkManagerPtr manager = new NetworkManager(context);
      XxxNetworkServerPtr server = new XxxManagerNetworkServer(context, manager);
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

class XxxExecutionContextCallback : public ExecutionContextCallback
{
public:
  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    std::cout << result.toString() << std::endl;
  }
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
      //remoteContext->pushWorkUnit(new DumbWorkUnit(), new XxxExecutionContextCallback());
      workUnits->addWorkUnit(new DumbWorkUnit());
    Variable result = remoteContext->run(workUnits);

    context.informationCallback(result.toString());
    //remoteContext->waitUntilAllWorkUnitsAreDone();
    return true;
  }

protected:
  friend class XxxClientWorkUnitClass;

  String hostName;
  size_t hostPort;
};

}; /* namespace */

#endif // !LBCPP_NETWORK_WORK_UNIT_H_
