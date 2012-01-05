/*-----------------------------------------.---------------------------------.
| Filename: LocalGridNetworkClient.cpp     | Local Grid Network Client       |
| Author  : Julien Becker                  |                                 |
| Started : 12/08/2011 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkClient.h>
#include <lbcpp/Network/NetworkMessage.h>

namespace lbcpp
{

class DecoratorExecutionContext : public ExecutionContext
{
public:
  DecoratorExecutionContext(ExecutionContext& decorated)
    : decorated(&decorated)
  {
    randomGenerator = decorated.getRandomGenerator();
    projectDirectory = decorated.getProjectDirectory();
    stack = decorated.getStack();
  }

  virtual bool isMultiThread() const
    {return decorated->isMultiThread();}

  virtual bool isCanceled() const
    {return decorated->isCanceled();}

  virtual bool isPaused() const
    {return isPaused();}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback = ExecutionContextCallbackPtr())
    {decorated->pushWorkUnit(workUnit, callback);}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone, bool pushIntoStack = true)
    {decorated->pushWorkUnit(workUnit, counterToDecrementWhenDone, pushIntoStack);}

  virtual void waitUntilAllWorkUnitsAreDone()
    {decorated->waitUntilAllWorkUnitsAreDone();}

  virtual void flushCallbacks()
    {decorated->flushCallbacks();}
  
  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack = true)
    {return run(workUnit, pushIntoStack);}

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack = true)
    {return run(workUnits, pushIntoStack);}


protected:
  ExecutionContext* decorated;
};

class LocalGridExecutionContextCallback : public ExecutionContextCallback
{
public:
  LocalGridExecutionContextCallback(ExecutionContext& context, const GridNetworkClientPtr& client, const String& uniqueIdentifier)
    : context(context), runContext(new DecoratorExecutionContext(context)), client(client), uniqueIdentifier(uniqueIdentifier), isFinished(false)
  {
    trace = new ExecutionTrace();
    runContext->appendCallback(makeTraceExecutionCallback(trace));
  }
  
  ExecutionContextPtr getExecutionContext() const
    {return runContext;}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    ExecutionTracesNetworkMessagePtr message = new ExecutionTracesNetworkMessage();
    message->addExecutionTrace(context, uniqueIdentifier, trace);
    client->sendVariable(message);
    isFinished = true;
  //  delete this;
  }

  void waitWorkUnitsAreDone() const
  {
    while (!isFinished)
    {
      juce::Thread::sleep(200);
    }
  }

protected:
  ExecutionContext& context;
  ExecutionContextPtr runContext;
  ExecutionTracePtr trace;
  GridNetworkClientPtr client;
  String uniqueIdentifier;
  volatile bool isFinished;
};

typedef LocalGridExecutionContextCallback* LocalGridExecutionContextCallbackPtr;

class LocalGridNetworkClient : public GridNetworkClient, public GridNetworkClientCallback
{
public:
  LocalGridNetworkClient(ExecutionContext& context)
    : GridNetworkClient(context), isWaitingWorkUnits(false) {callback = this;}
  
  virtual void workUnitRequestsReceived(const std::vector<WorkUnitNetworkRequestPtr>& workUnitRequests)
  {
    ScopedLock _(lock);

    std::vector<LocalGridExecutionContextCallbackPtr> callbacks(workUnitRequests.size());
    for (size_t i = 0; i < workUnitRequests.size(); ++i)
    {
      callbacks[i] = new LocalGridExecutionContextCallback(context, this, workUnitRequests[i]->getUniqueIdentifier());
      callbacks[i]->getExecutionContext()->pushWorkUnit(workUnitRequests[i]->getWorkUnit(context), callbacks[i]);
    }

    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->waitWorkUnitsAreDone();
      delete callbacks[i];
    }
    context.waitUntilAllWorkUnitsAreDone();

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

protected:
  CriticalSection lock;
  volatile bool isWaitingWorkUnits;
};

GridNetworkClientPtr localGridNetworkClient(ExecutionContext& context)
  {return new LocalGridNetworkClient(context);}
  
}; /* namespace lbcpp */
