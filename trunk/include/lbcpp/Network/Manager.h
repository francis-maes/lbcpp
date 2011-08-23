/*-----------------------------------------.---------------------------------.
| Filename: Manager.h                      |  Manager                        |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 13:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MANAGER_H_
# define LBCPP_MANAGER_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/WorkUnitNetworkRequest.h>

namespace lbcpp
{

class Manager
{
public:
  Manager(ExecutionContext& context)
    : context(context), lastUniqueIdentifier(Time::currentTimeMillis())
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
        
        requests[request->getUniqueIdentifier()] = request;
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
  
  void addRequest(const WorkUnitNetworkRequestPtr& request, const NetworkClientPtr& client)
  {
    ScopedLock _(lock);
    requests[request->getUniqueIdentifier()] = request;
    request->saveToFile(context, createFullPathOfFile(getRequestFile(context, request)));
    createFullPathOfFile(getWaitingFile(context, request));
    waitingRequests.push_back(request);
    routingTable[request->getUniqueIdentifier()] = client;
  }

  void archiveRequest(PairPtr archive)
  {
    ScopedLock _(lock);
    WorkUnitNetworkRequestPtr request = archive->getVariable(0).getObjectAndCast<WorkUnitNetworkRequest>(context);
    archive->saveToFile(context, createFullPathOfFile(getArchiveFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getUniqueIdentifier());
    routingTable.erase(request->getUniqueIdentifier());
  }

  void crachedRequest(WorkUnitNetworkRequestPtr request)
  {
    ScopedLock _(lock);
    request->saveToFile(context, createFullPathOfFile(getCrachedFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getUniqueIdentifier());
  }

  WorkUnitNetworkRequestPtr getRequest(const String& identifier) const
  {
    ScopedLock _(lock);
    std::map<String, WorkUnitNetworkRequestPtr>::const_iterator res = requests.find(identifier);
    if (res == requests.end())
    {
      context.warningCallback(T("Manager::getRequest"), T("The associated request of ") + identifier.quoted() + T(" not found !"));
      return WorkUnitNetworkRequestPtr();
    }
    return res->second;
  }

  void getWaitingRequests(const String& nodeName, std::vector<WorkUnitNetworkRequestPtr>& results)
  {
    ScopedLock _(lock);
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
    ScopedLock _(lock);
    for (size_t i = 0; i < networkRequests.size(); ++i)
    {
      waitingRequests.push_back(networkRequests[i]);
      createFullPathOfFile(getWaitingFile(context, networkRequests[i]));
    }
  }

  XmlElementPtr getXmlResult(const WorkUnitNetworkRequestPtr& request)
  {
    File f = getArchiveFile(context, request);
    if (!f.exists())
    {
      context.warningCallback(T("Manager::getXmlResult"), T("File not found: ") + f.getFileName());
      return XmlElementPtr();
    }
    
    PairPtr p = Pair::createFromFile(context, f);
    if (!p)
    {
      context.warningCallback(T("Manager::getXmlResult"), T("Corrupted file"));
      return XmlElementPtr();
    }

    XmlElementPtr xml = p->getSecond().getObjectAndCast<XmlElement>(context); // tag: lbcpp
    xml = getXmlChildElementOrWarning(xml, T("node"));
    if (!xml)
      return XmlElementPtr();
      
    return getXmlChildElementOrWarning(xml, T("return"));
  }

  XmlElementPtr getXmlChildElementOrWarning(const XmlElementPtr element, const String& tagName) const
  {
    XmlElementPtr child = element->getChildByName(tagName);
    if (!child)
      context.warningCallback(T("Manager::getXmlChildElementOrWarning"), T("Node ") + tagName.quoted() + T(" not found !"));
    return child;
  }

  String generateUniqueIdentifier()
  {
    ScopedLock _(indentifierLock);
    return String(++lastUniqueIdentifier); 


    juce::int64 res;
    do
    {
      juce::Thread::sleep(100);
      res = Time::currentTimeMillis();
    } while (res <= lastUniqueIdentifier);
    
    lastUniqueIdentifier = res;
    return String(res);
  }

  NetworkClientPtr getNetworkClientOf(const String& uniqueIdentifier) const
  {
    if (routingTable.count(uniqueIdentifier))
      return routingTable.find(uniqueIdentifier)->second;
    return NetworkClientPtr();
  }

  void setNetworkClientOf(const String& uniqueIndentifier, const NetworkClientPtr& client)
  {
    routingTable[uniqueIndentifier] = client;
  }

  void addNetworkClient(const NetworkClientPtr& client)
    {clients[client] = true;}

  void removeNetworkClient(const NetworkClientPtr& client)
  {
    if (clients.count(client) == 1)
      clients.erase(client);
  }

protected:
  ExecutionContext& context;
  CriticalSection lock;
  std::map<String, WorkUnitNetworkRequestPtr> requests; // id
  std::vector<WorkUnitNetworkRequestPtr> waitingRequests;
  std::map<String, NetworkClientPtr> routingTable; // uniqueIdentifier > network client
  std::map<NetworkClientPtr, bool> clients; // list of connected client

  static File createFullPathOfFile(const File& f)
  {
    f.getParentDirectory().createDirectory();
    f.create();
    return f;
  }
  
  File getRequestFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    return context.getFile(request->getProjectName() + T("/Requests/") + request->getUniqueIdentifier() + T(".request"));
  }

  File getArchiveFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    Time time(request->getUniqueIdentifier().getLargeIntValue());
    return context.getFile(request->getProjectName() + T("/") + time.formatted(T("%Y-%m-%d")) + T("/") + request->getUniqueIdentifier() + T(".archive"));
  }
  
  File getCrachedFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    Time time(request->getUniqueIdentifier().getLargeIntValue());
    return context.getFile(request->getProjectName() + T("/") + time.formatted(T("%Y-%m-%d")) + T("/Error/") + request->getUniqueIdentifier() + T(".request"));
  }

  File getWaitingFile(ExecutionContext& context, WorkUnitNetworkRequestPtr request) const
  {
    return context.getFile(request->getProjectName() + T(".") + request->getUniqueIdentifier());
  }

private:
  juce::int64 lastUniqueIdentifier;
  CriticalSection indentifierLock;
};

typedef Manager* ManagerPtr;

}; /* namespace */
  
#endif // !LBCPP_MANAGER_H_

