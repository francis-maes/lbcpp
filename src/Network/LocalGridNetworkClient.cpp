/*-----------------------------------------.---------------------------------.
| Filename: LocalGridNetworkClient.cpp     | Local Grid Network Client       |
| Author  : Julien Becker                  |                                 |
| Started : 12/08/2011 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkClient.h>

namespace lbcpp
{

class LocalGridExecutionContextCallback : public ExecutionContextCallback
{
public:
  LocalGridExecutionContextCallback(const GridNetworkClientPtr& client, const String& uniqueIdentifier)
    : client(client), uniqueIdentifier(uniqueIdentifier) {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    // TODO: send trace
    /*client->sendWorkUnitResult(uniqueIdentifier, result);*/
  }

protected:
  GridNetworkClientPtr client;
  String uniqueIdentifier;
};

class LocalGridNetworkClient : public GridNetworkClient, public GridNetworkClientCallback
{
public:
  LocalGridNetworkClient(ExecutionContext& context)
    : GridNetworkClient(context), lastWorkUnitId(0), isWaitingWorkUnits(false) {callback = this;}
  
  virtual void workUnitRequestsReceived(const std::vector<WorkUnitNetworkRequestPtr>& workUnitRequests)
  {
    ScopedLock _(lock);

    for (size_t i = 0; i < workUnitRequests.size(); ++i)
      context.pushWorkUnit(workUnitRequests[i]->getWorkUnit(context), new LocalGridExecutionContextCallback(this, String((int)++lastWorkUnitId)));

    isWaitingWorkUnits = false;
  }

  virtual void askForWorkUnits(const String& gridName)
  {
    isWaitingWorkUnits = true;
    sendVariable(new GetWaitingWorkUnitsNetworkMessage(gridName));
  }

  virtual void sendFinishedTraces() {}

  virtual void waitResponses(juce::int64 timeout)
  {
    static const juce::int64 timeToSleep = 500;
    while (isWaitingWorkUnits)
    {
      if (timeout < 0)
      {
        context.warningCallback(T("LocalGridNetworkClient::waitResponses"), T("Timeout"));
        return;
      }

      juce::Thread::sleep(timeToSleep);
      timeout -= timeToSleep;
    }

    context.waitUntilAllWorkUnitsAreDone();
  }

  virtual void closeCommunication()
  {
    sendVariable(new CloseCommunicationNetworkMessage());
  }

protected:
  CriticalSection lock;
  volatile size_t lastWorkUnitId;
  volatile bool isWaitingWorkUnits;
};

GridNetworkClientPtr localGridNetworkClient(ExecutionContext& context)
  {return new LocalGridNetworkClient(context);}
  
}; /* namespace lbcpp */
