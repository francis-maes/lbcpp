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
# include <lbcpp/Network/NetworkRequest.h>

namespace lbcpp
{

class NetworkInterface : public Object
{
public:
  NetworkInterface(ExecutionContext& context)
    : context(context) {}
  NetworkInterface(ExecutionContext& context, NetworkClientPtr client)
    : context(context), client(client) {}
  NetworkInterface()
    : context(*(ExecutionContext*)NULL) {}

  void setContext(ExecutionContext& context)
    {this->context = context;}

  ExecutionContext& getContext() const
    {return context;}

  void setNetworkClient(NetworkClientPtr client)
    {this->client = client;}

  NetworkClientPtr getNetworkClient() const
    {return client;}

  virtual void sendInterfaceClass()
    {client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(this));}

  virtual void closeCommunication()
    {client->stopClient();}

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

  String getNodeName() const
    {return nodeName;}

protected:
  friend class NodeNetworkInterfaceClass;
  
  String nodeName;
};

typedef ReferenceCountedObjectPtr<NodeNetworkInterface> NodeNetworkInterfacePtr;

class ManagerNodeNetworkInterface : public NodeNetworkInterface
{
public:
  ManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name)
    : NodeNetworkInterface(context, client, name) {}
  ManagerNodeNetworkInterface() {}

  virtual String pushWorkUnit(NetworkRequestPtr request) = 0;
  virtual bool isFinished(const String& identifier) = 0;
  virtual NetworkResponsePtr getExecutionTrace(const String& identifier) = 0;
};

typedef ReferenceCountedObjectPtr<ManagerNodeNetworkInterface> ManagerNodeNetworkInterfacePtr;

extern ManagerNodeNetworkInterfacePtr clientManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);

class GridNodeNetworkInterface : public NodeNetworkInterface
{
public:
  GridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
    : NodeNetworkInterface(context, client, nodeName) {}
  GridNodeNetworkInterface() {}

  // input : containerClass(networkRequestClass)
  // return: containerClass(stringType)
  // special return value in case of error: T("Error")
  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests) = 0;
  // return: containerClass(networkResponse)
  virtual ContainerPtr getFinishedExecutionTraces() = 0;
  // Normally not send on network
  virtual void removeExecutionTraces(ContainerPtr networkResponses) = 0;
};

typedef ReferenceCountedObjectPtr<GridNodeNetworkInterface> GridNodeNetworkInterfacePtr;

}; /* namespace */

#endif // !LBCPP_NETWORK_INTERFACE_H_
