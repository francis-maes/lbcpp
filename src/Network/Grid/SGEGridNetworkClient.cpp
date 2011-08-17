/*-----------------------------------------.---------------------------------.
| Filename: SGEGridNetworkClient.cpp       | SGE Grid Network Client         |
| Author  : Julien Becker                  |                                 |
| Started : 16/08/2011 10:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkClient.h>

namespace lbcpp
{

class SGEGridNetworkClient : public GridNetworkClient, public GridNetworkClientCallback
{
public:
  SGEGridNetworkClient(ExecutionContext& context)
    : GridNetworkClient(context), isWaitingWorkUnits(false), isSendingTraces(false)
  {
    callback = this;
    createDirectoryIfNotExists(T("Requests"));
    createDirectoryIfNotExists(T("PreProcessing"));
    createDirectoryIfNotExists(T("WorkUnits"));
    createDirectoryIfNotExists(T("Finished"));
    createDirectoryIfNotExists(T("Traces"));
    createDirectoryIfNotExists(T("Jobs"));
  }
  
  virtual void workUnitRequestsReceived(const std::vector<WorkUnitNetworkRequestPtr>& workUnitRequests)
  {
    ScopedLock _(lock);

    for (size_t i = 0; i < workUnitRequests.size(); ++i)
    {
      workUnitRequests[i]->saveToFile(context, getRequestFile(workUnitRequests[i]));
      workUnitRequests[i]->getXmlElementWorkUnit()->saveToFile(context, getWaitingFile(workUnitRequests[i]));
    }
    isWaitingWorkUnits = false;
  }

  virtual void askForWorkUnits(const String& gridName)
  {
    isWaitingWorkUnits = true;
    sendVariable(new GetWaitingWorkUnitsNetworkMessage(gridName));
  }

  virtual void sendFinishedTraces()
  {
    isSendingTraces = true;
    ExecutionTracesNetworkMessagePtr message = new ExecutionTracesNetworkMessage();
    StreamPtr files = directoryFileStream(context, getFinishDirectory(), T("*"));
    while (!files->isExhausted())
    {
      String identifier = files->next().getFile().getFileName();
      message->addExecutionTrace(context, identifier, getExecutionTrace(identifier));
    }

    if (message->getXmlElementExecutionTraces().size() == 0)
    {
      isSendingTraces = false;
      return;
    }

    if (sendVariable(message))
    {
      const std::vector< std::pair<String, XmlElementPtr> >& traces = message->getXmlElementExecutionTraces();
      for (size_t i = 0; i < traces.size(); ++i)
        removeExecutionTrace(traces[i].first);
    }
    else
      context.warningCallback(T("SGEGridNetworkClient::sendFinishedTraces"), T("Attemps to send finished execution traces failed ! Trace were not removed."));

    isSendingTraces = false;
  }

  virtual void waitResponses(juce::int64 timeout)
  {
    static const juce::int64 timeToSleep = 500;
    while (isWaitingWorkUnits || isSendingTraces)
    {
      if (timeout < 0)
      {
        context.warningCallback(T("SGEGridNetworkClient::waitResponses"), T("Timeout"));
        return;
      }

      juce::Thread::sleep(timeToSleep);
      timeout -= timeToSleep;
    }
  }

  virtual void closeCommunication()
  {
    sendVariable(new CloseCommunicationNetworkMessage());
  }

protected:
  CriticalSection lock;
  volatile bool isWaitingWorkUnits;
  volatile bool isSendingTraces;

  void createDirectoryIfNotExists(const String& directory)
  {
    File f = context.getFile(directory);
    if (!f.exists())
      f.createDirectory();
  }

  File getRequestFile(WorkUnitNetworkRequestPtr request)
    {return context.getFile(T("Requests/") + request->getUniqueIdentifier() + T(".request"));}
  
  File getWaitingFile(WorkUnitNetworkRequestPtr request)
    {return context.getFile(T("PreProcessing/") + request->getUniqueIdentifier() + T(".workUnit"));}
  
  File getFinishDirectory()
    {return context.getProjectDirectory().getChildFile(T("Finished"));}

  ExecutionTracePtr getExecutionTrace(const String& uniqueIdentifier)
  {
    File f = context.getFile(T("Traces/") + uniqueIdentifier + T(".trace"));
    if (!f.exists())
    {
      context.warningCallback(T("SGEGridNetworkClient::getExecutionTrace"), T("Trace not found: ") + f.getFileName());
      return ExecutionTracePtr();
    }
    return ExecutionTrace::createFromFile(context, f);
  }

  void removeExecutionTrace(const String& uniqueIdentifier)
  {
    File f = context.getFile(T("Traces/") + uniqueIdentifier + T(".trace"));
    f.deleteFile();
    f = context.getFile(T("Requests/") + uniqueIdentifier + T(".request"));
    f.deleteFile();
    f = context.getFile(T("Finished/") + uniqueIdentifier);
    f.deleteFile();
    f = context.getFile(T("WorkUnits/") + uniqueIdentifier + T(".workUnit"));
    f.deleteFile();
  }
};

GridNetworkClientPtr sgeGridNetworkClient(ExecutionContext& context)
  {return new SGEGridNetworkClient(context);}
  
}; /* namespace lbcpp */
