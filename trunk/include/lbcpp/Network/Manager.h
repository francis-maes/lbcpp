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

namespace lbcpp
{

class WorkUnitNetworkRequest : public Object
{
public:
  WorkUnitNetworkRequest(ExecutionContext& context,
                         WorkUnitPtr workUnit,
                         const String& projectName, const String& source, const String& destination,
                         size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
  : workUnit(new XmlElement()),
    projectName(projectName), source(source), destination(destination),
    requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime)
    {this->workUnit->saveObject(context, workUnit, T("workUnit"));}
  
 void setUniqueIdentifier(const String& uniqueIdentifier)
    {this->uniqueIdentifier = uniqueIdentifier;}
  
  const String& getUniqueIdentifier() const
    {return uniqueIdentifier;}

  const String& getProjectName() const
    {return projectName;}

  const String& getSource() const
    {return source;}

  const String& getDestination() const
    {return destination;}

  const size_t& getRequiredMemory() const
    {return requiredMemory;}

  const size_t& getRequiredTime() const
    {return requiredTime;}

  WorkUnitPtr getWorkUnit(ExecutionContext& context) const
    {return workUnit ? workUnit->createObjectAndCast<WorkUnit>(context) : WorkUnitPtr();}

  XmlElementPtr getXmlElementWorkUnit() const
    {return workUnit;}

protected:
  friend class WorkUnitNetworkRequestClass;

  WorkUnitNetworkRequest() {}

  String uniqueIdentifier;
  XmlElementPtr workUnit;
  String projectName;
  String source;
  String destination;
  size_t requiredCpus;
  size_t requiredMemory; // Gb
  size_t requiredTime; // Hour
};

typedef ReferenceCountedObjectPtr<WorkUnitNetworkRequest> WorkUnitNetworkRequestPtr;

class Manager
{
public:
  Manager(ExecutionContext& context)
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
  
  void addRequest(WorkUnitNetworkRequestPtr request)
  {
    ScopedLock _(lock);
    requests[request->getUniqueIdentifier()] = request;
    request->saveToFile(context, createFullPathOfFile(getRequestFile(context, request)));
    createFullPathOfFile(getWaitingFile(context, request));
    waitingRequests.push_back(request);
  }

  void archiveRequest(PairPtr archive)
  {
    ScopedLock _(lock);
    WorkUnitNetworkRequestPtr request = archive->getVariable(0).getObjectAndCast<WorkUnitNetworkRequest>(context);
    archive->saveToFile(context, createFullPathOfFile(getArchiveFile(context, request)));
    getRequestFile(context, request).deleteFile();
    requests.erase(request->getUniqueIdentifier());
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
      return WorkUnitNetworkRequestPtr();
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

  Variable getResult(const String& uniqueIdentifier)
  {
    jassertfalse;
    return false;
  }

  String generateUniqueIdentifier()
  {
    ScopedLock _(indentifierLock);
    juce::int64 res;
    do
    {
      juce::Thread::sleep(1);
      res = Time::currentTimeMillis();
    } while (res <= lastUniqueIdentifier);
    
    lastUniqueIdentifier = res;
    return String(res);
  }

protected:
  ExecutionContext& context;
  CriticalSection lock;
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

typedef ReferenceCountedObjectPtr<Manager> ManagerPtr;

}; /* namespace */
  
#endif // !LBCPP_MANAGER_H_

