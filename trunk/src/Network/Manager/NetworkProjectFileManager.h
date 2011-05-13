/*-----------------------------------------.---------------------------------.
| Filename: NetworkProjectFileManager.h    | Network Project File Manager    |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 13:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_PROJECT_FILE_MANAGER_H_
# define LBCPP_NETWORK_PROJECT_FILE_MANAGER_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Network/WorkUnitNetworkRequest.h>

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
        WorkUnitNetworkRequestPtr request = WorkUnitNetworkRequest::createFromFile(context, f);
        if (!request)
        {
          context.errorCallback(T("ManagerNetworkInterface::ManagerNetworkInterface"), T("Fail to restore: ") + f.getFileName());
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
  
  void addRequest(WorkUnitNetworkRequestPtr request)
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
    WorkUnitNetworkRequestPtr request = archive->getWorkUnitNetworkRequest();
    archive->saveToFile(context, createFullPathOfFile(getArchiveFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getIdentifier());
  }
  
  void crachedRequest(WorkUnitNetworkRequestPtr request)
  {
//    ScopedLock _(globalLock);
    request->saveToFile(context, createFullPathOfFile(getCrachedFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getIdentifier());
  }

  WorkUnitNetworkRequestPtr getRequest(const String& identifier) const
  {
//    ScopedLock _(globalLock);
    std::map<String, WorkUnitNetworkRequestPtr>::const_iterator res = requests.find(identifier);
    if (res == requests.end())
      return WorkUnitNetworkRequestPtr();
    return res->second;
  }
  
  void getWaitingRequests(const String& nodeName, std::vector<WorkUnitNetworkRequestPtr>& results)
  {
//    ScopedLock _(globalLock);
    std::vector<WorkUnitNetworkRequestPtr> remainingRequests;
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
  
  void setAsWaitingRequests(const std::vector<WorkUnitNetworkRequestPtr>& networkRequests)
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
  std::map<String, WorkUnitNetworkRequestPtr> requests; // id
  std::vector<WorkUnitNetworkRequestPtr> waitingRequests;

  static File createFullPathOfFile(const File& f)
  {
    f.getParentDirectory().createDirectory();
    f.create();
    return f;
  }
  
  File getRequestFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    return context.getFile(request->getProjectName() + T("/Requests/") + request->getIdentifier() + T(".request"));
  }
  
  File getArchiveFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    Time time(request->getCreationTime());
    return context.getFile(request->getProjectName() + T("/") + time.formatted(T("%Y-%m-%d")) + T("/") + request->getIdentifier() + T(".archive"));
  }
  
  File getCrachedFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    Time time(request->getCreationTime());
    return context.getFile(request->getProjectName() + T("/") + time.formatted(T("%Y-%m-%d")) + T("/Error/") + request->getIdentifier() + T(".request"));
  }

  File getWaitingFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    return context.getFile(request->getProjectName() + T(".") + request->getIdentifier());
  }
};

typedef ReferenceCountedObjectPtr<NetworkProjectFileManager> NetworkProjectFileManagerPtr;

}; /* namespace */
  
#endif // !LBCPP_NETWORK_PROJECT_FILE_MANAGER_H_

