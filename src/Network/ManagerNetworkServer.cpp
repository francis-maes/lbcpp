/*-----------------------------------------.---------------------------------.
| Filename: ManagerNetworkServer.cpp       | Manager Network Server          |
| Author  : Julien Becker                  |                                 |
| Started : 12/08/2011 12:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkServer.h>

/** ManagerServerNetworkClient **/
namespace lbcpp
{

class ManagerServerNetworkClient : public NetworkClient
{
public:
  ManagerServerNetworkClient(ExecutionContext& context, const ManagerPtr& manager)
    : NetworkClient(context), manager(manager) {}

  /** Message handler **/
  virtual void variableReceived(const Variable& variable)
  {
    if (!isValidNetworkMessage(context, variable))
      return;
    
    const ObjectPtr obj = variable.getObject();
    const ClassPtr objClass = obj->getClass();
    if (objClass == workUnitRequestNetworkMessageClass)
    {
      WorkUnitRequestNetworkMessagePtr message = obj.staticCast<WorkUnitRequestNetworkMessage>();
      workUnitRequestReceived(message->getSourceIdentifier(), message->getWorkUnitNetworkRequest());
    }
    else if (objClass == getWorkUnitResultNetworkMessageClass)
    {
      GetWorkUnitResultNetworkMessagePtr message = obj.staticCast<GetWorkUnitResultNetworkMessage>();
      getWorkUnitResultReceived(message->getUniqueIdentifier());
    }
    else if (objClass == getWaitingWorkUnitsNetworkMessageClass)
    {
      GetWaitingWorkUnitsNetworkMessagePtr message = obj.staticCast<GetWaitingWorkUnitsNetworkMessage>();
      getWaitingWorkUnitsReceived(message->getGridName());
    }
    else if (objClass == executionTracesNetworkMessageClass)
    {
      ExecutionTracesNetworkMessagePtr message = obj.staticCast<ExecutionTracesNetworkMessage>();
      const std::vector< std::pair<String, XmlElementPtr> >& traces = message->getXmlElementExecutionTraces();
      for (size_t i = 0; i < traces.size(); ++i)
        xmlExecutionTracesReceived(traces[i].first, traces[i].second);
    }
    else if (objClass == closeCommunicationNetworkMessageClass)
    {
      closeCommunicationReceived();
    }
    else
      context.warningCallback(T("ManagerServerNetworkClient::variableReceived")
                              , T("Unknwon object of type: ") + objClass->toString());
  }

  /** Receiver **/
  void workUnitRequestReceived(size_t sourceIdentifier, const WorkUnitNetworkRequestPtr& workUnitRequest)
  {
    String uniqueIdentifier = manager->generateUniqueIdentifier();
    bool isAlreadyExists = false;
    {
      ScopedLock _(lock);
      isAlreadyExists = sourceToUniqueIdentifier.count(sourceIdentifier) != 0;
      if (isAlreadyExists)
        uniqueIdentifier = sourceToUniqueIdentifier[sourceIdentifier];
      else
        sourceToUniqueIdentifier[sourceIdentifier] = uniqueIdentifier;
    }

    if (isAlreadyExists)
      context.warningCallback(T("ManagerServerNetworkClient::workUnitRequestReceived")
                              , T("I still received this request, so I return you the same unique identifier"));
    else
      manager->addRequest(workUnitRequest);

    sendWorkUnitAcknowledgement(sourceIdentifier, uniqueIdentifier);
  }
  
  void getWorkUnitResultReceived(const String& uniqueIdentifier)
  {
    sendWorkUnitResult(uniqueIdentifier, manager->getXmlResult(uniqueIdentifier));
  }

  void getWaitingWorkUnitsReceived(const String& gridName)
  {
    // TODO: Use a CompositeWorkUnit ??
    std::vector<WorkUnitNetworkRequestPtr> results;
    manager->getWaitingRequests(gridName, results);
    sendWorkUnits(results);
    // TODO: No acknowledgement needed ...
  }

  void xmlExecutionTracesReceived(const String& uniqueIdentifier, const XmlElementPtr& xmlTrace)
  {
    WorkUnitNetworkRequestPtr request = manager->getRequest(uniqueIdentifier);
    if (!request)
    {
      context.warningCallback(T("ManagerServerNetworkClient::xmlExecutionTracesReceived")
                              , T("I received trace but I didn't find associated WorkUnit ! So I skip !"));
      return;
    }
    manager->archiveRequest(new Pair(request, xmlTrace));
  }

  void closeCommunicationReceived()
  {
    juce::Thread::sleep(3000);
    stopClient();
  }
  
  /** Sender **/
  bool sendWorkUnitAcknowledgement(size_t sourceIdentifier, const String& uniqueIdentifier)
  {
    return sendVariable(new WorkUnitAcknowledgementNetworkMessage(sourceIdentifier, uniqueIdentifier));
  }

  bool sendWorkUnitResult(const String& uniqueIdentifier, const XmlElementPtr& result)
  {
    return sendVariable(new WorkUnitResultNetworkMessage(context, uniqueIdentifier, result));
  }

  bool sendWorkUnits(const std::vector<WorkUnitNetworkRequestPtr>& workUnitRequests)
  {
    return sendVariable(new WorkUnitRequestsNetworkMessage(workUnitRequests));
  }

protected:
  ManagerPtr manager;
  
  CriticalSection lock;
  std::map<size_t, String> sourceToUniqueIdentifier;
};

/** ManagerNetworkServer **/
class ManagerNetworkServer : public NetworkServer
{
public:
  ManagerNetworkServer(ExecutionContext& context, ManagerPtr manager)
    : NetworkServer(context), manager(manager) {}

  virtual NetworkClient* createNetworkClient()
    {return new ManagerServerNetworkClient(context, manager);}

  lbcpp_UseDebuggingNewOperator

protected:
  ManagerPtr manager;
};

NetworkServerPtr managerNetworkServer(ExecutionContext& context, const ManagerPtr& manager)
  {return new ManagerNetworkServer(context, manager);}

}; /* namespace lbcpp */
