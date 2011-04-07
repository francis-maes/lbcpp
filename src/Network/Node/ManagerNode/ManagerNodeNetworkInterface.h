/*-----------------------------------------.---------------------------------.
| Filename: ManagerNodeNetworkInterface.h  | Manager Node Network Interface  |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MANAGER_NODE_NETWORK_INTERFACE_H_
# define LBCPP_MANAGER_NODE_NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkInterface.h>
# include "NetworkProjectFileManager.h"

namespace lbcpp
{

class ClientManagerNodeNetworkInterface : public ManagerNodeNetworkInterface
{
public:
  ClientManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName = String::empty)
    : ManagerNodeNetworkInterface(context, client, nodeName)  {}
  ClientManagerNodeNetworkInterface() {}

  virtual String pushWorkUnit(NetworkRequestPtr request);
  virtual bool isFinished(const String& identifier);
  virtual NetworkResponsePtr getExecutionTrace(const String& identifier);

  /* NetworkInterface */
  virtual void closeCommunication();
};

extern ClassPtr clientManagerNodeNetworkInterfaceClass;

class FileSystemManagerNodeNetworkInterface : public ManagerNodeNetworkInterface
{
public:
  FileSystemManagerNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name, NetworkProjectFileManagerPtr fileManager)
    : ManagerNodeNetworkInterface(context, client, name), fileManager(fileManager) {}
  FileSystemManagerNodeNetworkInterface() {}

  virtual String pushWorkUnit(NetworkRequestPtr request)
  {
    request->selfGenerateIdentifier();
    fileManager->addRequest(request);
    return request->getIdentifier();
  }

  virtual bool isFinished(const String& identifier)
    {return fileManager->getRequest(identifier) == NetworkRequestPtr();} // light test

  virtual NetworkResponsePtr getExecutionTrace(const String& identifier)
  {
    juce::OwnedArray<File> results;
    if (!context.getProjectDirectory().findChildFiles(results, File::findFiles, true, identifier + T(".archive")))
      return new NetworkResponse(identifier);
    if (results.size() != 1)
      context.warningCallback(T("FileSystemManagerNodeNetworkInterface::getExecutionTrace"), T("Many traces found for: ") + identifier);
    NetworkArchivePtr archive = NetworkArchive::createFromFile(context, *results[0]);
    return archive->getNetworkResponse();
  }

protected:
  NetworkProjectFileManagerPtr fileManager;
};

};

#endif // !LBCPP_MANAGER_NODE_NETWORK_INTERFACE_H_
