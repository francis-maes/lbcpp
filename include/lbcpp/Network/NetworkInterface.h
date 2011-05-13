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
# include <lbcpp/Network/WorkUnitNetworkRequest.h>

namespace lbcpp
{

class NetworkInterface : public NameableObject
{
public:
  NetworkInterface(ExecutionContext& context, const String& name = String::empty)
    : NameableObject(name), context(context) {}
  NetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name = String::empty)
    : NameableObject(name), context(context), client(client) {}
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

class ManagerNetworkInterface : public NetworkInterface
{
public:
  ManagerNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name)
    : NetworkInterface(context, client, name) {}
  ManagerNetworkInterface() {}

  virtual String pushWorkUnit(WorkUnitNetworkRequestPtr request) = 0;
  virtual bool isFinished(const String& identifier) = 0;
  virtual ExecutionTraceNetworkResponsePtr getExecutionTrace(const String& identifier) = 0;
};

typedef ReferenceCountedObjectPtr<ManagerNetworkInterface> ManagerNetworkInterfacePtr;

extern ManagerNetworkInterfacePtr clientManagerNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);

class GridNetworkInterface : public NetworkInterface
{
public:
  GridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
    : NetworkInterface(context, client, nodeName) {}
  GridNetworkInterface() {}

  // input : containerClass(networkRequestClass)
  // return: containerClass(stringType)
  // special return value in case of error: T("Error")
  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests) = 0;
  // return: containerClass(networkResponse)
  virtual ContainerPtr getFinishedExecutionTraces() = 0;
  // Normally not send on network
  virtual void removeExecutionTraces(ContainerPtr networkResponses) = 0;
};

typedef ReferenceCountedObjectPtr<GridNetworkInterface> GridNetworkInterfacePtr;

}; /* namespace */

#endif // !LBCPP_NETWORK_INTERFACE_H_
