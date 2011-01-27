/*-----------------------------------------.---------------------------------.
| Filename: NetworkContext.h               | Network Context                 |
| Author  : Julien Becker                  |                                 |
| Started : 17/12/2010 16:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_CONTEXT_H_
# define LBCPP_NETWORK_CONTEXT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Network/NetworkClient.h>

namespace lbcpp
{

class NetworkContext : public WorkUnit
{
public:
  NetworkContext(const String& identity, const String& hostname, int port)
  : identity(identity), hostname(hostname), port(port) {}
  NetworkContext() {}
  
  // Etablish a connection to server and execute commands
  virtual Variable run(ExecutionContext& context);
  
  NetworkClientPtr getNetworkClient() const
    {return client;}

  void stopClient()
    {client->stopClient();}
  
  String getIdentity() const
    {return identity;}
  
protected:
  friend class NetworkContextClass;
  
  String identity;
  String hostname;
  int port;
  
  NetworkClientPtr client;
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
    if (!workUnits.size())
      return WorkUnitPtr();
    WorkUnitPtr res = workUnits.front();
    workUnits.pop_front();
    return res;
  }
  
  void submittedWorkUnit(juce::int64 workUnitId, WorkUnitPtr workUnit)
    {submittedWorkUnits.push_back(std::make_pair<juce::int64, WorkUnitPtr>(workUnitId, workUnit));}
  
protected:
  std::deque<WorkUnitPtr> workUnits;
  std::vector<std::pair<juce::int64, WorkUnitPtr> > submittedWorkUnits;
  CriticalSection lock;
};

class ServerNetworkContext : public NetworkContext
{
public:
  ServerNetworkContext(const String& identity, const String& hostname, int port)
  : NetworkContext(identity, hostname, port) {}
  ServerNetworkContext() {}
  
  virtual String getWorkUnitStatus(ExecutionContext& context, juce::int64 workUnitId) const = 0;

  virtual void pushWorkUnit(ExecutionContext& context, juce::int64 workUnitId, WorkUnitPtr workUnit) = 0;
  
  virtual Variable getWorkUnitTrace(ExecutionContext& context, juce::int64 workUnitId) const = 0;
};

class SgeServerNetworkContext : public ServerNetworkContext
{
public:
  SgeServerNetworkContext(const String& identity, const String& hostname, int port, File workUnitDirectory)
  : ServerNetworkContext(identity, hostname, port), workUnitDirectory(workUnitDirectory) {}
  SgeServerNetworkContext() {}
  
  virtual String getWorkUnitStatus(ExecutionContext& context, juce::int64 workUnitId) const;
  virtual void pushWorkUnit(ExecutionContext& context, juce::int64 workUnitId, WorkUnitPtr workUnit);
  virtual Variable getWorkUnitTrace(ExecutionContext& context, juce::int64 workUnitId) const;
  
protected:
  friend class SgeServerNetworkContextClass;
  
  File workUnitDirectory;
};

extern ClassPtr clientNetworkContextClass;
extern ClassPtr serverNetworkContextClass;

typedef ReferenceCountedObjectPtr<NetworkContext> NetworkContextPtr;
typedef ReferenceCountedObjectPtr<ClientNetworkContext> ClientNetworkContextPtr;
typedef ReferenceCountedObjectPtr<ServerNetworkContext> ServerNetworkContextPtr;

};

#endif // !LBCPP_NETWORK_CONTEXT_H_
