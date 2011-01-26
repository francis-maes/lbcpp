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
  
class NetworkCommand : public Object
{
public:
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network) = 0;
};

typedef ReferenceCountedObjectPtr<NetworkCommand> NetworkCommandPtr;

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
  GetWorkUnitStatusNetworkCommand(juce::int64 workUnitId = 0) : workUnitId(workUnitId) {}
  
  virtual bool runCommand(ExecutionContext& context, NetworkContextPtr network)
  {
    ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
    if (!serverNetwork)
    {
      context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"), T("Must be execute in a ServerNetworkContext only"));
      return false;
    }
    
    network->getNetworkClient()->sendVariable(serverNetwork->getWorkUnitStatus(context, workUnitId));
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
