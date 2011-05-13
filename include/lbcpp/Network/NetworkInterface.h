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
  NetworkInterface()
    : context(*(ExecutionContext*)NULL) {}

  void setContext(ExecutionContext& context)
    {this->context = context;}

  ExecutionContext& getContext() const
    {return context;}

protected:
  friend class NetworkInterfaceClass;

  ExecutionContext& context;
};

typedef ReferenceCountedObjectPtr<NetworkInterface> NetworkInterfacePtr;

template<class BaseClass>
class ForwarderNetworkInterface : public BaseClass
{
public:
  ForwarderNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name)
    : BaseClass(context, name), client(client) {}
  ForwarderNetworkInterface() {}
  
  void setNetworkClient(NetworkClientPtr client)
    {this->client = client;}
  
  NetworkClientPtr getNetworkClient() const
    {return client;}

protected:
  NetworkClientPtr client;
};

class ManagerNetworkInterface : public NetworkInterface
{
public:
  ManagerNetworkInterface(ExecutionContext& context, const String& name)
    : NetworkInterface(context, name) {}
  ManagerNetworkInterface() {}

  virtual String pushWorkUnit(WorkUnitNetworkRequestPtr request) = 0;
  virtual bool isFinished(const String& identifier) = 0;
  virtual ExecutionTraceNetworkResponsePtr getExecutionTrace(const String& identifier) = 0;
};

typedef ReferenceCountedObjectPtr<ManagerNetworkInterface> ManagerNetworkInterfacePtr;

extern ManagerNetworkInterfacePtr forwarderManagerNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);

class GridNetworkInterface : public NetworkInterface
{
public:
  GridNetworkInterface(ExecutionContext& context, const String& nodeName)
    : NetworkInterface(context, nodeName) {}
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
