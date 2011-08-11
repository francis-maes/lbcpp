/*-----------------------------------------.---------------------------------.
| Filename: DistributedExecutionContext.h  | Distributed Execution           |
| Author  : Julien Bekcer                  | Context                         |
| Started : 10/08/2011 09:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_DISTRIBUTED_H_
# define LBCPP_EXECUTION_CONTEXT_DISTRIBUTED_H_

# include "SubExecutionContext.h"
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkMessage.h>

namespace lbcpp
{

class WorkUnitPool : public Object
{
public:
  WorkUnitPool()
    : numFinishedWorkUnits(0) {}

  void waitUntilAllWorkUnitsAreDone()
  {
    while (workUnits.size() != numFinishedWorkUnits)
      juce::Thread::sleep(1000);
  }

  void appendWorkUnit(size_t internalId, const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback)
  {
    ScopedLock _(lock);
    indices[internalId] = workUnits.size();
    workUnits.push_back(workUnit);
    results.push_back(String::empty);
    callbacks.push_back(callback);
  }

  void setResult(size_t internalId, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(indices.count(internalId) && indices[internalId] < results.size());
    const size_t index = indices[internalId];
    results[index] = result;

    if (callbacks[index])
      callbacks[index]->workUnitFinished(workUnits[index], result);

    ++numFinishedWorkUnits;
  }

  Variable getResult() const
  {
    jassert(workUnits.size() == results.size());
    jassert(workUnits.size() == numFinishedWorkUnits);
    const size_t n = results.size();
    if (n == 1)
      return results[0];

    VectorPtr res = variableVector(n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, results[i]);
    return res;
  }

protected:
  CriticalSection lock;

  volatile size_t numFinishedWorkUnits;
  std::map<size_t, size_t> indices; // Internal ID -> index
  std::vector<WorkUnitPtr> workUnits;
  std::vector<Variable> results;
  std::vector<ExecutionContextCallbackPtr> callbacks;
};

typedef ReferenceCountedObjectPtr<WorkUnitPool> WorkUnitPoolPtr;

class XxxDistributedExecutionContextNetworkClient : public XxxManagerNetworkClient, public XxxManagerNetworkClientCallback
{
public:
  XxxDistributedExecutionContextNetworkClient(ExecutionContext& context)
    : XxxManagerNetworkClient(context, this), numSentWorkUnit(0)
    {}

  virtual void workUnitAcknowledgementReceived(size_t sourceIdentifier, const String& uniqueIdentifier)
  {
    ScopedLock _(lock);
    jassert(sourceIdentifier < numSentWorkUnit);
    // Update identifier mapping
    workUnitIds[uniqueIdentifier] = sourceIdentifier;
  }

  virtual void workUnitResultReceived(const String& uniqueIdentifier, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(workUnitIds.count(uniqueIdentifier));
    const size_t internalId = workUnitIds[uniqueIdentifier];
    jassert(pools.count(internalId));
    WorkUnitPoolPtr pool = pools[internalId];
    pool->setResult(internalId, result);

    workUnitIds.erase(uniqueIdentifier);
    pools.erase(internalId);
  }

  virtual bool sendWorkUnit(const WorkUnitPtr& workUnit, size_t sourceIdentifier)
  {
    return sendVariable(new WorkUnitRequestNetworkMessage(context, sourceIdentifier, workUnit));
  }

  bool sendWorkUnit(const WorkUnitPtr& workUnit, const WorkUnitPoolPtr& pool, const ExecutionContextCallbackPtr& callback = ExecutionContextCallbackPtr())
  {
    ScopedLock _(lock);
    pool->appendWorkUnit(numSentWorkUnit, workUnit, callback);
    pools[numSentWorkUnit] = pool;
    return sendWorkUnit(workUnit, numSentWorkUnit++);
  }

protected:
  CriticalSection lock;

  size_t numSentWorkUnit;
  std::map<String, size_t> workUnitIds; // Manager ID to interne ID
  std::map<size_t, WorkUnitPoolPtr> pools;
};

typedef ReferenceCountedObjectPtr<XxxDistributedExecutionContextNetworkClient> XxxDistributedExecutionContextNetworkClientPtr;

class DistributedExecutionContext : public SubExecutionContext
{
public:
  DistributedExecutionContext(ExecutionContext& parentContext, const String& remoteHostName, size_t remotePort)
    : SubExecutionContext(parentContext)
    , client(new XxxDistributedExecutionContextNetworkClient(parentContext))
    , defaultPool(new WorkUnitPool())
    {
      client->startClient(remoteHostName, remotePort);
    }

  DistributedExecutionContext() {}

  virtual ~DistributedExecutionContext()
  {
    client->stopClient();
  }

  virtual String toString() const
    {return T("Distributed(") + parent->toString() + T(")");}

  virtual bool isMultiThread() const
    {return true;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback = ExecutionContextCallbackPtr())
  {
    client->sendWorkUnit(workUnit, defaultPool, callback);
  }
  
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
  {
    // No mechanisme to retreive result
    client->sendWorkUnit(workUnit, defaultPool);
  }

  virtual void waitUntilAllWorkUnitsAreDone()
    {defaultPool->waitUntilAllWorkUnitsAreDone();}
    
  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack)
  {
    WorkUnitPoolPtr pool = new WorkUnitPool();
    client->sendWorkUnit(workUnit, pool);
    pool->waitUntilAllWorkUnitsAreDone();
    return pool->getResult();
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    WorkUnitPoolPtr pool = new WorkUnitPool();
    const size_t n = workUnits->getNumWorkUnits();
    for (size_t i = 0; i < n; ++i)
      client->sendWorkUnit(workUnits->getWorkUnit(i), pool);
    pool->waitUntilAllWorkUnitsAreDone();
    return pool->getResult();
  }

  lbcpp_UseDebuggingNewOperator

protected:
  XxxDistributedExecutionContextNetworkClientPtr client;
  WorkUnitPoolPtr defaultPool;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_DISTRIBUTED_H_
