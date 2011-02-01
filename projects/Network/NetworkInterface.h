/*-----------------------------------------.---------------------------------.
| Filename: NetworkInterface.h             | Network Interface               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_INTERFACE_H_
# define LBCPP_NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Execution/ExecutionTrace.h>
# include "NetworkRequest.h"

namespace lbcpp
{

class NetworkInterface : public Object
{
public:
  NetworkInterface(ExecutionContext& context) : context(context) {}
  NetworkInterface(ExecutionContext& context, NetworkClientPtr client) : context(context), client(client) {}
  NetworkInterface() : context(*(ExecutionContext*)NULL) {}
  
  void setContext(ExecutionContext& context)
    {this->context = context;}
  
  ExecutionContext& getContext() const
    {return context;}
  
  void setNetworkClient(NetworkClientPtr client)
    {this->client = client;}
  
  NetworkClientPtr getNetworkClient() const
    {return client;}
  
  virtual void sendInterfaceClass();
  
  virtual void closeCommunication(ExecutionContext& context);
  
protected:
  friend class NetworkInterfaceClass;
  
  ExecutionContext& context;
  NetworkClientPtr client;
};

typedef ReferenceCountedObjectPtr<NetworkInterface> NetworkInterfacePtr;

class NodeNetworkInterface : public NetworkInterface
{
public:
  NodeNetworkInterface(ExecutionContext& context, const String& nodeName = String::empty)
  : NetworkInterface(context), nodeName(nodeName) {}
  NodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName = String::empty)
  : NetworkInterface(context, client), nodeName(nodeName) {}
  NodeNetworkInterface() {}
  
  virtual String getNodeName(ExecutionContext& context) const
    {return nodeName;}
  
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr workUnit) = 0;
  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr workUnit) const = 0;
  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr workUnit) const = 0;
  
protected:
  String nodeName;
};

typedef ReferenceCountedObjectPtr<NodeNetworkInterface> NodeNetworkInterfacePtr;

/*
** NodeNetworkInterface - Implementation
*/

class ClientNodeNetworkInterface : public NodeNetworkInterface
{
public:
  ClientNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName = String::empty);
  ClientNodeNetworkInterface() {}
  
  virtual void closeCommunication(ExecutionContext& context);
  
  virtual String getNodeName(ExecutionContext& context) const;
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request);
  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const;
  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const;
};


class SgeNodeNetworkInterface : public NodeNetworkInterface
{
public:
  SgeNodeNetworkInterface(ExecutionContext& context, const String& nodeName = String::empty);
  SgeNodeNetworkInterface() {}
  
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request);
  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const;
  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const;
};


class ManagerNodeNetworkInterface : public NodeNetworkInterface
{
public:
  ManagerNodeNetworkInterface(ExecutionContext& context);
  ManagerNodeNetworkInterface() {}
  
  virtual void closeCommunication(ExecutionContext& context);
  
  virtual NetworkRequestPtr pushWorkUnit(ExecutionContext& context, WorkUnitNetworkRequestPtr request);
  virtual int getWorkUnitStatus(ExecutionContext& context, NetworkRequestPtr request) const;
  virtual ExecutionTracePtr getExecutionTrace(ExecutionContext& context, NetworkRequestPtr request) const;
  
  void getRequestsSentTo(const String& nodeName, std::vector<NetworkRequestPtr>& results) const;
  WorkUnitNetworkRequestPtr getWorkUnit(ExecutionContext& context, NetworkRequestPtr request) const;
  
protected:
  friend class ManagerNodeNetworkInterfaceClass;

  std::vector<NetworkRequestPtr> requests;

  void createDirectoryIfNotExists(ExecutionContext& context, const String& directoryName);
};

typedef ReferenceCountedObjectPtr<ManagerNodeNetworkInterface> ManagerNodeNetworkInterfacePtr;

extern ClassPtr clientNodeNetworkInterfaceClass;
extern ClassPtr nodeNetworkInterfaceClass;

}; /* namespace */

#endif // !LBCPP_NETWORK_INTERFACE_H_
