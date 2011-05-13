/*-----------------------------------------.---------------------------------.
| Filename: ManagerNetworkInterface.h      | Manager Network Interface       |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MANAGER_NETWORK_INTERFACE_H_
# define LBCPP_MANAGER__NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkInterface.h>
# include "NetworkProjectFileManager.h"

namespace lbcpp
{

class ClientManagerNetworkInterface : public ManagerNetworkInterface
{
public:
  ClientManagerNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& Name = String::empty)
    : ManagerNetworkInterface(context, client, Name)  {}
  ClientManagerNetworkInterface() {}

  virtual String pushWorkUnit(WorkUnitNetworkRequestPtr request);
  virtual bool isFinished(const String& identifier);
  virtual ExecutionTraceNetworkResponsePtr getExecutionTrace(const String& identifier);

  /* NetworkInterface */
  virtual void closeCommunication();
};

extern ClassPtr clientManagerNetworkInterfaceClass;

class FileSystemManagerNetworkInterface : public ManagerNetworkInterface
{
public:
  FileSystemManagerNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& name, NetworkProjectFileManagerPtr fileManager)
    : ManagerNetworkInterface(context, client, name), fileManager(fileManager) {}
  FileSystemManagerNetworkInterface() {}

  virtual String pushWorkUnit(WorkUnitNetworkRequestPtr request)
  {
    request->selfGenerateIdentifier();
    fileManager->addRequest(request);
    return request->getIdentifier();
  }

  virtual bool isFinished(const String& identifier)
    {return fileManager->getRequest(identifier) == WorkUnitNetworkRequestPtr();} // light test

  virtual ExecutionTraceNetworkResponsePtr getExecutionTrace(const String& identifier)
  {
    juce::OwnedArray<File> results;
    if (!context.getProjectDirectory().findChildFiles(results, File::findFiles, true, identifier + T(".archive")))
      return new ExecutionTraceNetworkResponse(identifier);
    if (results.size() != 1)
      context.warningCallback(T("FileSystemManagerNetworkInterface::getExecutionTrace"), T("Many traces found for: ") + identifier);
    NetworkArchivePtr archive = NetworkArchive::createFromFile(context, *results[0]);
    return archive->getExecutionTraceNetworkResponse();
  }

protected:
  NetworkProjectFileManagerPtr fileManager;
};

};

#endif // !LBCPP_MANAGER_NETWORK_INTERFACE_H_
