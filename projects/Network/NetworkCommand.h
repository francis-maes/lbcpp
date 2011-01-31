/*-----------------------------------------.---------------------------------.
| Filename: NetworkCommand.h               | Network Command                 |
| Author  : Julien Becker                  |                                 |
| Started : 02/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_COMMAND_H_
# define LBCPP_NETWORK_COMMAND_H_

# include <lbcpp/lbcpp.h>
# include "NetworkContext.h"

namespace lbcpp
{

class NetworkRequest : public Object
{
public:
  enum {communicationError, unknown, waitingOnManager, waitingOnServer, running, finished, iDontHaveThisWorkUnit};
  
  NetworkRequest(const String& identifier, const String& projectName, const String& source, const String& destination)
    : identifier(identifier), projectName(projectName), source(source), destination(destination), status(unknown) {}
  NetworkRequest() {}
  
  String getIdentifier() const
    {return identifier;}
  
  String getProjectName() const
    {return projectName;}
  
  String getSource() const
    {return source;}
  
  String getDestination() const
    {return destination;}
  
  int getStatus() const
    {return status;}
  
  void setStatus(int status)
    {this->status = status;}
  
  void selfGenerateIdentifier()
    {identifier = generateIdentifier();}
  
protected:
  String identifier;
  String projectName;
  String source;
  String destination;
  int status;

  static juce::int64 lastIdentifier;
  
  String generateIdentifier()
  {
    juce::int64 res = Time::currentTimeMillis();
    if (res != lastIdentifier)
    {
      lastIdentifier = res;
      return String(res);
    }
    juce::Thread::sleep(1);
    return generateIdentifier();
  }
};

typedef ReferenceCountedObjectPtr<NetworkRequest> NetworkRequestPtr;
  
class WorkUnitNetworkRequest : public NetworkRequest
{
public:
  WorkUnitNetworkRequest(WorkUnitPtr workUnit, const String& projetName, const String& source, const String& destination)
    : NetworkRequest(generateIdentifier(), projectName, source, destination), workUnit(workUnit) {}
  WorkUnitNetworkRequest() {}

  WorkUnitPtr getWorkUnit() const
    {return workUnit;}

  NetworkRequestPtr getNetworkRequest() const
    {return new NetworkRequest(getIdentifier(), getProjectName(), getSource(), getDestination());}

protected:
  WorkUnitPtr workUnit;
};

typedef ReferenceCountedObjectPtr<WorkUnitNetworkRequest> WorkUnitNetworkRequestPtr;

/*
** NetworkInterface
*/
class NetworkInterface : public Object
{
public:
  NetworkInterface() : context(*(ExecutionContext*)NULL) {}
  
  void setContext(ExecutionContext& context)
    {this->context = context;}
  
  ExecutionContext& getContext() const
    {return context;}
  
  void setNetworkClient(NetworkClientPtr client)
    {this->client = client;}
  
  NetworkClientPtr getNetworkClient() const
    {return client;}
  
  virtual void sendInterfaceClass()
    {client->sendVariable(getClass());}
  
  virtual void closeCommunication()
    {jassertfalse;}
  
  virtual String getNodeName() const
  {jassertfalse; return T("");}
  
protected:
  ExecutionContext& context;
  NetworkClientPtr client;
};

typedef ReferenceCountedObjectPtr<NetworkInterface> NetworkInterfacePtr;

class NodeNetworkInterface : public NetworkInterface
{
public:
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr workUnit) = 0;
  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr workUnit) const = 0;
  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr workUnit) const = 0;
};

typedef ReferenceCountedObjectPtr<NodeNetworkInterface> NodeNetworkInterfacePtr;


/*
** NetworkNotification
*/
class NetworkNotification : public Notification
{
public:
  virtual void notify(const ObjectPtr& target)
    {notifyNetwork(target);}

  virtual void notifyNetwork(const NetworkInterfacePtr& target) = 0;
};

class NodeNetworkNotification : public NetworkNotification
{
  virtual void notifyNetwork(const NetworkInterfacePtr& target)
  {
    //if (target->getClass()->inheritsFrom(NodeNetworkNotificationClass))
      notifyNodeNetwork(target);
    NetworkNotification::notifyNetwork(target);
  }
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target) = 0;
};

class PushWorkUnitNotification : public NodeNetworkNotification
{
public:
  PushWorkUnitNotification(WorkUnitNetworkRequestPtr request) : request(request) {}

  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
  {
    bool res = target->pushWorkUnit(target->getContext(), request);
    target->getNetworkClient()->sendVariable(Variable(res, booleanType));
  }

protected:
  WorkUnitNetworkRequestPtr request;
};

class GetWorkUnitStatusNotification : public NodeNetworkNotification
{
public:
  GetWorkUnitStatusNotification(NetworkRequestPtr request) : request(request) {}

  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
  {
    int res = target->getWorkUnitStatus(target->getContext(), request);
    target->getNetworkClient()->sendVariable(Variable(res, integerType));
  }

protected:
  NetworkRequestPtr request;
};

class GetExecutionTraceNotification : public NodeNetworkNotification
{
public:
  GetExecutionTraceNotification(NetworkRequestPtr request) : request(request) {}
  
  virtual void notifyNodeNetwork(const NodeNetworkInterfacePtr& target)
  {
    ExecutionTracePtr res = target->getExecutionTrace(target->getContext(), request);
    target->getNetworkClient()->sendVariable(res);
  }
  
protected:
  NetworkRequestPtr request;
};


class ClientNodeNetworkInterface : public NodeNetworkInterface
{
public:
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request)
  {
    client->sendVariable(new PushWorkUnitNotification(request));
    NetworkRequestPtr res = false;
    if (!client->receiveObject<NetworkRequest>(10000, res))
      context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::pushWorkUnit"));
    return res;
  }
  
  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
  {
    client->sendVariable(new GetWorkUnitStatusNotification(request));
    int res = NetworkRequest::communicationError;
    if (!client->receiveInteger(10000, res))
      context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getWorkUnitStatut"));
    return res;
  }
  
  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
  {
    client->sendVariable(new GetExecutionTraceNotification(request));
    ExecutionTracePtr res;
    if (!client->receiveObject<ExecutionTrace>(10000, res))
      context.warningCallback(client->getConnectedHostName(), T("ClientNodeNetworkInterface::getExecutionTrace"));
    return res;
  }
};


class SgeNodeNetworkInterface : public NodeNetworkInterface
{
public:
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request)
  {
    File f = context.getFile(T("Waiting/") + request->getIdentifier() + T(".workUnit"));
    request->getWorkUnit()->saveToFile(context, f);
    return request->getNetworkRequest();
  }

  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
  {
    File f = context.getFile(T("Waiting/") + request->getIdentifier() + T(".workUnit"));
    if (f.exists())
      return NetworkRequest::waitingOnServer;
    f = context.getFile(T("InProgress/") + request->getIdentifier() + T(".workUnit"));
    if (f.exists())
      return NetworkRequest::running;
    f = context.getFile(T("Finished/") + request->getIdentifier() + T(".workUnit"));
    if (f.exists())
      return NetworkRequest::finished;
    return NetworkRequest::iDontHaveThisWorkUnit;
  }

  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
  {
    File f = context.getFile(T("Traces/") + request->getIdentifier() + T(".trace"));
    if (!f.exists())
      return ExecutionTracePtr();
    
    return Variable::createFromFile(context, f).getObject();
  }
};


class ManagerNodeNetworkInterface : public NodeNetworkInterface
{
public:
  ManagerNodeNetworkInterface()
  {
    // ... restore requests
  }

  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request)
  {
    request->selfGenerateIdentifier();
    request->setStatus(NetworkRequest::waitingOnManager);
    /* First, backup request */
    NetworkRequestPtr res = request->getNetworkRequest();
    File f = context.getFile(T("Requests/") + res->getIdentifier() + T(".request"));
    res->saveToFile(context, f);
    f = context.getFile(T("WorkUnits/") + res->getIdentifier() + T(".workUnit"));
    request->getWorkUnit()->saveToFile(context, f);

    requests.push_back(res);
    return res;
  }

  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const
  {
    for (size_t i = 0; i < requests.size(); ++i)
      if (requests[i]->getIdentifier() == request->getIdentifier())
        return requests[i]->getStatus();
  }

  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const
  {
    File f = context.getFile(T("Traces/") + request->getIdentifier() + T(".trace"));
    if (!f.exists())
      return ExecutionTracePtr();
    return Object::createFromFile(context, f);
  }
  
  void getRequestsSentTo(const String& nodeName, std::vector<NetworkRequestPtr>& results) const
  {
    jassertfalse;
  }
  
  WorkUnitNetworkRequestPtr getWorkUnit(ExecutionContext& context, NetworkRequestPtr request) const
  {
    File f = context.getFile(T("WorkUnits/") + request->getIdentifier() + T(".workUnit"));
    return Object::createFromFile(context, f);
  }

protected:
  std::vector<NetworkRequestPtr> requests;
};

typedef ReferenceCountedObjectPtr<ManagerNodeNetworkInterface> ManagerNodeNetworkInterfacePtr;

class ManagerWorkUnit : public WorkUnit
{
public:
  ManagerWorkUnit() : port(1664) {}

  virtual Variable run(ExecutionContext& context)
  {
    NetworkServerPtr server = new NetworkServer(context);
    if (!server->startServer(port))
    {
      context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port ") + String((int)port));
      return false;
    }
    
    ManagerNodeNetworkInterfacePtr managerInterface = new ManagerNodeNetworkInterface();
    
    while (true)
    {
      /* Accept client */
      NetworkClientPtr client = server->acceptClient(INT_MAX);
      if (!client)
        continue;
      context.informationCallback(client->getConnectedHostName(), T("connected"));
      
      /* Which kind of connection ? */
      ClassPtr type;
      if (!client->receiveObject<Class>(10000, type))
      {
        context.warningCallback(client->getConnectedHostName(), T("Fail - Client type (1)"));
        client->stopClient();
        continue;
      }
      
      /* Strat communication (depending of the type) */
      NetworkInterfacePtr interface;
      //if (type->inheritsFrom(clientNodeNetworkInterfaceClass))
      {
        interface = managerInterface;
        interface->setContext(context);
        interface->setNetworkClient(client);

        serverCommunication(interface);
      }
      //else if (type->inheritsFrom(nodeNetworkInterfaceClass))
      {
        interface = new ClientNodeNetworkInterface();
        interface->setContext(context);
        interface->setNetworkClient(client);

        clientCommunication(interface);
      }
      //else
      {
        interface = new NetworkInterface();
        interface->setContext(context);
        interface->setNetworkClient(client);
      }

      /* Terminate the connection */
      interface->closeCommunication();
      context.informationCallback(client->getConnectedHostName(), T("disconnected"));
    }
  }

protected:
  size_t port;
  
  void serverCommunication(NodeNetworkInterfacePtr interface) const
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
  
  void clientCommunication(ManagerNodeNetworkInterfacePtr interface) const
  {
    String nodeName = interface->getNodeName();
    if (nodeName == String::empty)
    {
      interface->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("Fail - Empty node name"));
      return;
    }
    
    /* Update status */
    std::vector<NetworkRequestPtr> requests;
    interface->getRequestsSentTo(nodeName, requests);
    for (size_t i = 0; i < requests.size(); ++i)
    {
      int status = interface->getWorkUnitStatus(interface->getContext(), requests[i]);
      if (status == NetworkRequest::iDontHaveThisWorkUnit) // implicitly send new request
        interface->pushWorkUnit(interface->getContext(), interface->getWorkUnit(interface->getContext(), requests[i]));
      int oldStatus = requests[i]->getStatus();
      requests[i]->setStatus(status);
      // On change : Save request
      if (oldStatus != status)
      {
        File f = interface->getContext().getFile(T("Requests/") + requests[i]->getIdentifier() + T(".request"));
        requests[i]->saveToFile(interface->getContext(), f);
      }
    }
  }
};

  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
class NetworkCommand : public Object
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network) = 0;
};

typedef ReferenceCountedObjectPtr<NetworkCommand> NetworkCommandPtr;

class ClientNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkCommandPtr network);

protected:
  virtual bool runCommand(ExecutionContext& context, ClientNetworkContextPtr network) = 0;
};

class ServerNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network);

protected:
  virtual bool runCommand(ExecutionContext& context, ServerNetworkContextPtr network) = 0;
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

class GetWorkUnitNetworkCommand : public NetworkCommand
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    String hostname = network->getNetworkClient()->getConnectedHostName();
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

    String stringId;
    if (!network->getNetworkClient()->receiveString(10000, stringId))
    {
      context.warningCallback(T("GetWorkUnitNetworkCommand::runCommand"), T("Fail - WorkUnitId"));
      return false;
    }

    juce::int64 workUnitId = stringId.getLargeIntValue();
    clientNetwork->submittedWorkUnit(workUnitId, workUnit);
    context.informationCallback(hostname, T("WorkUnit Submitted - ID: ") + stringId);

    return true;
  }
};

class GetWorkUnitStatusNetworkCommand : public NetworkCommand
{
public:
  GetWorkUnitStatusNetworkCommand(juce::int64 workUnitId = 0) : workUnitIdString(workUnitId) {}
  
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }
    juce::int64 workUnitId = workUnitIdString.getLargeIntValue();
    network->getNetworkClient()->sendVariable(serverNetwork->getWorkUnitStatus(context, workUnitId));
    return true;
  }

protected:
  friend class GetWorkUnitStatusNetworkCommandClass;
  
  String workUnitIdString;
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
  : workUnitIdString(workUnitId), workUnit(workUnit) {}
  PushWorkUnitNetworkCommand() {}
  
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }
    juce::int64 workUnitId = workUnitIdString.getLargeIntValue();
    serverNetwork->pushWorkUnit(context, workUnitId, workUnit);
    return true;
  }
  
protected:
  friend class PushWorkUnitNetworkCommandClass;
  
  String workUnitIdString;
  WorkUnitPtr workUnit;
};

class SendTraceNetworkCommand : public NetworkCommand
{
public:
  SendTraceNetworkCommand(juce::int64 workUnitId) : workUnitIdString(workUnitId) {}
  SendTraceNetworkCommand() {}
  
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }
    juce::int64 workUnitId = workUnitIdString.getLargeIntValue();
    Variable trace = serverNetwork->getWorkUnitTrace(context, workUnitId);
    serverNetwork->getNetworkClient()->sendVariable(trace);
    return true;
  }
  
protected:
  friend class SendTraceNetworkCommandClass;
  
  String workUnitIdString;
};

}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_COMMAND_H_
