/*-----------------------------------------.---------------------------------.
| Filename: ManagerNodeNetworkInterface.h  | Manager Node Network Interface  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MANAGER_NODE_NETWORK_INTERFACE_H_
# define LBCPP_MANAGER_NODE_NETWORK_INTERFACE_H_

# include "../NodeNetworkInterface.h"
# include "NetworkProjectFileManager.h"

namespace lbcpp
{

class ManagerNodeNetworkInterface : public NodeNetworkInterface
{
public:
  ManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name)
    : NodeNetworkInterface(context, client, name) {}
  ManagerNodeNetworkInterface() {}

  virtual String pushWorkUnit(NetworkRequestPtr request) = 0;
};

typedef ReferenceCountedObjectPtr<ManagerNodeNetworkInterface> ManagerNodeNetworkInterfacePtr;

class ClientManagerNodeNetworkInterface : public ManagerNodeNetworkInterface
{
public:
  ClientManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName = String::empty)
    : ManagerNodeNetworkInterface(context, client, nodeName)  {}
  ClientManagerNodeNetworkInterface() {}

  virtual String pushWorkUnit(NetworkRequestPtr request);

  /* NetworkInterface */
  virtual void closeCommunication();
};

extern ClassPtr clientManagerNodeNetworkInterfaceClass;

class FileSystemManagerNodeNetworkInterface : public NodeNetworkInterface
{
public:
  FileSystemManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name, NetworkProjectFileManagerPtr fileManager)
    : NodeNetworkInterface(context, client, name), fileManager(fileManager) {}
  FileSystemManagerNodeNetworkInterface() {}

  virtual String pushWorkUnit(NetworkRequestPtr request)
  {
    request->selfGenerateIdentifier();
    fileManager->addRequest(request);
    return request->getIdentifier();
  }

protected:
  NetworkProjectFileManagerPtr fileManager;
};

};

#endif // !LBCPP_MANAGER_NODE_NETWORK_INTERFACE_H_
