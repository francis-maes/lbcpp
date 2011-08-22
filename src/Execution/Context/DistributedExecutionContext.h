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

# ifndef LBCPP_NETWORKING

class DistributedExecutionContext : public SubExecutionContext
{
public:
  DistributedExecutionContext(ExecutionContext& parentContext, const String& remoteHostName, size_t remotePort) {}
};

# else /* LBCPP_NETWORKING */
#  include <lbcpp/Network/NetworkClient.h>
#  include <lbcpp/Network/NetworkMessage.h>

namespace lbcpp
{

class WorkUnitPool : public Object
{
public:
  WorkUnitPool(bool isCompositeWorkUnit = false)
    : numFinishedWorkUnits(0) {}

  void waitUntilAllWorkUnitsAreDone()
  {
    while (entries.size() != numFinishedWorkUnits)
      juce::Thread::sleep(1000);
  }

  void appendWorkUnit(size_t internalId, const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback)
  {
    ScopedLock _(lock);
    indices[internalId] = entries.size();
    entries.push_back(Entry(workUnit, callback));
  }

  void setResult(size_t internalId, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(indices.count(internalId) && indices[internalId] < entries.size());
    const size_t index = indices[internalId];
    entries[index].result = result;

    if (entries[index].callback)
      entries[index].callback->workUnitFinished(entries[index].workUnit, result);

    ++numFinishedWorkUnits;
  }

  Variable getResult() const
  {
    jassert(entries.size() == numFinishedWorkUnits);
    const size_t n = entries.size();
    if (n == 1 && !isCompositeWorkUnit)
      return entries[0].result;

    VectorPtr res = variableVector(n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, entries[i].result);
    return res;
  }

protected:
  struct Entry
  {
    Entry(const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback)
      : workUnit(workUnit), callback(callback) {}
    
    WorkUnitPtr workUnit;
    Variable result;
    ExecutionContextCallbackPtr callback;
  };

  CriticalSection lock;

  bool isCompositeWorkUnit;
  volatile size_t numFinishedWorkUnits;
  std::map<size_t, size_t> indices; // Internal ID -> index

  std::vector<Entry> entries;
};

typedef ReferenceCountedObjectPtr<WorkUnitPool> WorkUnitPoolPtr;

class DistributedExecutionContextClientCallback : public ManagerNetworkClientCallback
{
public:
  DistributedExecutionContextClientCallback(ExecutionContext& context)
    : client(new ManagerNetworkClient(context)), numSentWorkUnit(0)
  {
    client->setCallback(this);
  }
  
  NetworkClientPtr getNetworkClient() const
    {return client;}

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

  bool sendWorkUnit(const WorkUnitPtr& workUnit, const WorkUnitPoolPtr& pool, const ExecutionContextCallbackPtr& callback,
                    const String& project, const String& from, const String& to)
  {
    ScopedLock _(lock);
    pool->appendWorkUnit(numSentWorkUnit, workUnit, callback);
    pools[numSentWorkUnit] = pool;
    bool res = client->sendWorkUnit(numSentWorkUnit++, workUnit, project, from, to, 1, 2, 10);
    return res;
  }

  virtual void connectionMade()
  {
    // FIXME: Doesn't work
    ScopedLock _(lock);
    for (std::map<String, size_t>::iterator it = workUnitIds.begin(); it != workUnitIds.end(); ++it)
      client->sendVariable(new GetWorkUnitResultNetworkMessage(it->first));
  }

protected:
  CriticalSection lock;

  ManagerNetworkClientPtr client;

  size_t numSentWorkUnit;
  std::map<String, size_t> workUnitIds; // Manager ID to interne ID
  std::map<size_t, WorkUnitPoolPtr> pools;
};

typedef DistributedExecutionContextClientCallback* DistributedExecutionContextClientCallbackPtr;

class DistributedExecutionContext : public SubExecutionContext
{
public:
  DistributedExecutionContext(ExecutionContext& parentContext, const String& remoteHostName, size_t remotePort,
                              const String& project, const String& from, const String& to)
    : SubExecutionContext(parentContext)
    , client(new DistributedExecutionContextClientCallback(parentContext))
    , defaultPool(new WorkUnitPool(false))
    , project(project), from(from), to(to)
    {
      client->getNetworkClient()->startClient(remoteHostName, remotePort);
    }

  DistributedExecutionContext() {}

  virtual ~DistributedExecutionContext()
  {
    client->getNetworkClient()->stopClient();
    delete client;
  }

  virtual String toString() const
    {return T("Distributed(") + parent->toString() + T(")");}

  virtual bool isMultiThread() const
    {return true;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, ExecutionContextCallbackPtr callback = NULL, bool pushIntoStack = true)
  {
    // TODO: pushIntoStack is not taken into account
    client->sendWorkUnit(workUnit, defaultPool, callback, project, from, to);
  }
  
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
  {
    jassertfalse;
  }

  virtual void waitUntilAllWorkUnitsAreDone()
    {defaultPool->waitUntilAllWorkUnitsAreDone();}
    
  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack)
  {
    WorkUnitPoolPtr pool = new WorkUnitPool(false);
    client->sendWorkUnit(workUnit, pool, ExecutionContextCallbackPtr(), project, from, to);
    pool->waitUntilAllWorkUnitsAreDone();
    return pool->getResult();
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    WorkUnitPoolPtr pool = new WorkUnitPool(true);
    const size_t n = workUnits->getNumWorkUnits();
    for (size_t i = 0; i < n; ++i)
      client->sendWorkUnit(workUnits->getWorkUnit(i), pool, ExecutionContextCallbackPtr(), project, from, to);
    pool->waitUntilAllWorkUnitsAreDone();
    return pool->getResult();
  }

  lbcpp_UseDebuggingNewOperator

protected:
  DistributedExecutionContextClientCallbackPtr client;
  WorkUnitPoolPtr defaultPool;

  String project;
  String from;
  String to;
};

}; /* namespace lbcpp */
# endif /* !LBCPP_NETWORKING */
#endif //!LBCPP_EXECUTION_CONTEXT_DISTRIBUTED_H_
