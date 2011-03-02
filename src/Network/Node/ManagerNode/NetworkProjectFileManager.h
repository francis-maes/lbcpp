/*-----------------------------------------.---------------------------------.
| Filename: NetworkProjectFileManager.h    | Network Project File Manager    |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 13:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NODE_NETWORK_PROJECT_FILE_MANAGER_H_
# define LBCPP_NODE_NETWORK_PROJECT_FILE_MANAGER_H_

# include "../NetworkRequest.h"
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class NetworkProjectFileManager
{
public:
  NetworkProjectFileManager(ExecutionContext& context)
    : context(context)
  {
    juce::OwnedArray<File> projectDirectories;
    context.getProjectDirectory().findChildFiles(projectDirectories, File::findDirectories, false, T("*"));
    
    for (size_t i = 0; i < (size_t)projectDirectories.size(); ++i)
    {
      context.enterScope(T("Loading project: ") + projectDirectories[i]->getFileName());
      StreamPtr files = directoryFileStream(context, projectDirectories[i]->getChildFile(T("Requests")), T("*.request"));
      while (!files->isExhausted())
      {
        File f = files->next().getFile();
        NetworkRequestPtr request = NetworkRequest::createFromFile(context, f);
        if (!request)
        {
          context.errorCallback(T("ManagerNodeNetworkInterface::ManagerNodeNetworkInterface"), T("Fail to restore: ") + f.getFileName());
          continue;
        }
        
        requests[request->getIdentifier()] = request;
        if (getWaitingFile(context, request).exists())
        {
          context.informationCallback(T("Waiting request restored: ") + f.getFileNameWithoutExtension());
          waitingRequests.push_back(request);
        }
        else
          context.informationCallback(T("Request restored: ") + f.getFileNameWithoutExtension());
      }
      context.leaveScope(Variable());
    }
  }
  
  void addRequest(NetworkRequestPtr request)
  {
//    ScopedLock _(globalLock);
    requests[request->getIdentifier()] = request;
    request->saveToFile(context, createFullPathOfFile(getRequestFile(context, request)));
    createFullPathOfFile(getWaitingFile(context, request));
    waitingRequests.push_back(request);
  }

  void archiveRequest(NetworkArchivePtr archive)
  {
//    ScopedLock _(globalLock);
    NetworkRequestPtr request = archive->getNetworkRequest();
    archive->saveToFile(context, createFullPathOfFile(getArchiveFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getIdentifier());
  }
  
  void crachedRequest(NetworkRequestPtr request)
  {
//    ScopedLock _(globalLock);
    request->saveToFile(context, createFullPathOfFile(getCrachedFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getIdentifier());
  }

  NetworkRequestPtr getRequest(const String& identifier) const
  {
//    ScopedLock _(globalLock);
    std::map<String, NetworkRequestPtr>::const_iterator res = requests.find(identifier);
    if (res == requests.end())
      return NetworkRequestPtr();
    return res->second;
  }
  
  void getWaitingRequests(const String& nodeName, std::vector<NetworkRequestPtr>& results)
  {
//    ScopedLock _(globalLock);
    std::vector<NetworkRequestPtr> remainingRequests;
    for (size_t i = 0; i < waitingRequests.size(); ++i)
    {
      if (waitingRequests[i]->getDestination() == nodeName)
      {
        results.push_back(waitingRequests[i]);
        getWaitingFile(context, waitingRequests[i]).deleteFile();
      }
      else
        remainingRequests.push_back(waitingRequests[i]);
    }
    waitingRequests = remainingRequests;
  }
  
  void setAsWaitingRequests(const std::vector<NetworkRequestPtr>& networkRequests)
  {
//    ScopedLock _(globalLock);
    for (size_t i = 0; i < networkRequests.size(); ++i)
    {
      waitingRequests.push_back(networkRequests[i]);
      createFullPathOfFile(getWaitingFile(context, networkRequests[i]));
    }
  }
  
protected:
  ExecutionContext& context;
  CriticalSection globalLock;
  std::map<String, NetworkRequestPtr> requests; // id
  std::vector<NetworkRequestPtr> waitingRequests;

  static File createFullPathOfFile(const File& f)
  {
    f.getParentDirectory().createDirectory();
    f.create();
    return f;
  }
  
  File getRequestFile(ExecutionContext& context, NetworkRequestPtr request) const
  {
    return context.getFile(request->getProjectName() + T("/Requests/") + request->getIdentifier() + T(".request"));
  }
  
  File getArchiveFile(ExecutionContext& context, NetworkRequestPtr request) const
  {
    Time time(request->getCreationTime());
    return context.getFile(request->getProjectName() + T("/") + time.formatted(T("%Y-%m-%d")) + T("/") + request->getIdentifier() + T(".archive"));
  }
  
  File getCrachedFile(ExecutionContext& context, NetworkRequestPtr request) const
  {
    Time time(request->getCreationTime());
    return context.getFile(request->getProjectName() + T("/") + time.formatted(T("%Y-%m-%d")) + T("/Error/") + request->getIdentifier() + T(".request"));
  }

  File getWaitingFile(ExecutionContext& context, NetworkRequestPtr request) const
  {
    return context.getFile(request->getProjectName() + T(".") + request->getIdentifier());
  }
};

typedef ReferenceCountedObjectPtr<NetworkProjectFileManager> NetworkProjectFileManagerPtr;

}; /* namespace */
  
#endif // !LBCPP_NODE_NETWORK_PROJECT_FILE_MANAGER_H_

