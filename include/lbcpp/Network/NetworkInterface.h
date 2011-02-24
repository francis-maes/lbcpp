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

  virtual WorkUnitInformationPtr pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request) = 0;
  virtual WorkUnitInformation::Status getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const = 0;
  virtual NetworkResponsePtr getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const = 0;

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

  /* NetworkInterface */
  virtual void closeCommunication(ExecutionContext& context);

  /* NodeNetworkInterface */
  virtual String getNodeName(ExecutionContext& context) const;
  virtual WorkUnitInformationPtr pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request);
  virtual WorkUnitInformation::Status getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const;
  virtual NetworkResponsePtr getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const;
};


class SgeNodeNetworkInterface : public NodeNetworkInterface
{
public:
  SgeNodeNetworkInterface(ExecutionContext& context, const String& nodeName = String::empty);
  SgeNodeNetworkInterface() {}

  virtual WorkUnitInformationPtr pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request);
  virtual WorkUnitInformation::Status getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const;
  virtual NetworkResponsePtr getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const;
};

class ManagerNodeNetworkInterface : public NodeNetworkInterface
{
public:
  ManagerNodeNetworkInterface(ExecutionContext& context);
  ManagerNodeNetworkInterface() {}

  /* NetworkInterface */
  virtual void closeCommunication(ExecutionContext& context);

  /* NodeNetworkInterface */
  virtual WorkUnitInformationPtr pushWorkUnit(ExecutionContext& context, NetworkRequestPtr request);
  virtual WorkUnitInformation::Status getWorkUnitStatus(ExecutionContext& context, WorkUnitInformationPtr information) const;
  virtual NetworkResponsePtr getExecutionTrace(ExecutionContext& context, WorkUnitInformationPtr information) const;

  /* ManagerNodeNetworkInterface */
  void getUnfinishedRequestsSentTo(const String& nodeName, std::vector<NetworkRequestPtr>& results) const;
  void archiveTrace(ExecutionContext& context, const NetworkRequestPtr& request, const NetworkResponsePtr& trace);

protected:
  friend class ManagerNodeNetworkInterfaceClass;

  File requestDirectory;
  File archiveDirectory;
  std::vector<NetworkRequestPtr> requests;
};

typedef ReferenceCountedObjectPtr<ManagerNodeNetworkInterface> ManagerNodeNetworkInterfacePtr;

extern ClassPtr clientNodeNetworkInterfaceClass;
extern ClassPtr nodeNetworkInterfaceClass;

}; /* namespace */

#endif // !LBCPP_NETWORK_INTERFACE_H_
