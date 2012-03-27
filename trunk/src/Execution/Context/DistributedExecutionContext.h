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
# include <list>

namespace lbcpp
{

class WorkUnitPool : public Object
{
public:
  virtual bool areAllWorkUnitsDone() const = 0;
  virtual void resultReceived(size_t index, const Variable& result, bool waitTrace) = 0;
  virtual void traceReceived(size_t index, const ExecutionTracePtr& trace) = 0;
};

typedef ReferenceCountedObjectPtr<WorkUnitPool> WorkUnitPoolPtr;

class SingleWorkUnitPool : public WorkUnitPool
{
public:
  SingleWorkUnitPool() : done(false) {}

  virtual void resultReceived(size_t index, const Variable& result, bool waitTrace)
  {
    jassert(index == 0);
    ScopedLock _(resultLock);
    this->result = result;
    if (!waitTrace)
      done = true;
  }

  virtual void traceReceived(size_t index, const ExecutionTracePtr& trace)
  {
    jassert(index == 0);
    ScopedLock _(resultLock);
    this->trace = trace;
    done = true;
  }

  virtual bool areAllWorkUnitsDone() const
    {return done;}

  const Variable& getResult() const
    {ScopedLock _(resultLock); return result;}

protected:
  volatile bool done;

  CriticalSection resultLock;
  Variable result;
  ExecutionTracePtr trace;
};

typedef ReferenceCountedObjectPtr<SingleWorkUnitPool> SingleWorkUnitPoolPtr;

class CompositeWorkUnitPool : public WorkUnitPool
{
public:
  CompositeWorkUnitPool(const CompositeWorkUnitPtr& workUnit)
    : numWorkUnitsDone(0), results(variableVector(workUnit->getNumWorkUnits())),
      traces(std::vector<ExecutionTracePtr>(workUnit->getNumWorkUnits())) {}

  virtual void resultReceived(size_t index, const Variable& result, bool waitTrace)
  {
    ScopedLock _(resultsLock);
    results->setElement(index, result);
    if (!waitTrace)
      ++numWorkUnitsDone;
  }

  virtual void traceReceived(size_t index, const ExecutionTracePtr& trace)
  {
    ScopedLock _(resultsLock);
    traces[index] = trace;
    ++numWorkUnitsDone;
  }

  virtual bool areAllWorkUnitsDone() const
    {return numWorkUnitsDone == results->getNumElements();}

  VariableVectorPtr getResults() const
    {ScopedLock _(resultsLock); return results;}
  
protected:
  volatile size_t numWorkUnitsDone;
  CriticalSection resultsLock;
  VariableVectorPtr results;
  std::vector<ExecutionTracePtr> traces;
};

typedef ReferenceCountedObjectPtr<CompositeWorkUnitPool> CompositeWorkUnitPoolPtr;

class AsynchroneousWorkUnitPool : public WorkUnitPool
{
public:
  void workUnitSent(size_t index, const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback)
  {
    ScopedLock _(sentWorkUnitsLock);
    jassert(sentWorkUnits.find(index) == sentWorkUnits.end());
    sentWorkUnits[index] = Entry(workUnit, callback);
  }

  virtual void resultReceived(size_t index, const Variable& result, bool waitTrace)
  {
    Entry entry;
    {
      // pop entry from sentWorkUnits
      ScopedLock _(sentWorkUnitsLock);
      std::map<size_t, Entry>::iterator it = sentWorkUnits.find(index);
      jassert(it != sentWorkUnits.end());

      if (!waitTrace)
      {
        entry = it->second;
        sentWorkUnits.erase(it);
        entry.result = result;
      }
      else
        it->second.result = result;
    }
    if (!waitTrace)
    {
      // push into finishedWorkUnitsLock
      ScopedLock _(finishedWorkUnitsLock);
      finishedWorkUnits.push_back(entry);
    }
  }

  virtual void traceReceived(size_t index, const ExecutionTracePtr& trace)
  {
    Entry entry;
    {
      // pop entry from sentWorkUnits
      ScopedLock _(sentWorkUnitsLock);
      std::map<size_t, Entry>::iterator it = sentWorkUnits.find(index);
      jassert(it != sentWorkUnits.end());

      entry = it->second;
      sentWorkUnits.erase(it);
    }
    entry.trace = trace;
    {
      // push into finishedWorkUnitsLock
      ScopedLock _(finishedWorkUnitsLock);
      finishedWorkUnits.push_back(entry);
    }
  }

  virtual bool areAllWorkUnitsDone() const
  {
    {
      ScopedLock _(sentWorkUnitsLock);
      if (sentWorkUnits.size())
        return false;
    }
    {
      ScopedLock _(finishedWorkUnitsLock);
      if (finishedWorkUnits.size())
        return false;
    }
    return true;
  }

  void flushCallbacks()
  {
    std::list<Entry> entries;
    {
      ScopedLock _(finishedWorkUnitsLock);
      finishedWorkUnits.swap(entries);
    }
    for (std::list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it)
      it->callback->workUnitFinished(it->workUnit, it->result, it->trace);
  }

protected:
  struct Entry
  {
    Entry(const WorkUnitPtr& workUnit, const ExecutionContextCallbackPtr& callback)
      : workUnit(workUnit), callback(callback) {}
    Entry() {}
    
    WorkUnitPtr workUnit;
    Variable result;
    ExecutionContextCallbackPtr callback;
    ExecutionTracePtr trace;
  };

  CriticalSection sentWorkUnitsLock;
  std::map<size_t, Entry> sentWorkUnits;

  CriticalSection finishedWorkUnitsLock;
  std::list<Entry> finishedWorkUnits;
};

typedef ReferenceCountedObjectPtr<AsynchroneousWorkUnitPool> AsynchroneousWorkUnitPoolPtr;

class DistributedExecutionContext : public SubExecutionContext, public ManagerNetworkClientCallback
{
public:
  DistributedExecutionContext(ExecutionContext& parentContext,
                              const String& remoteHostName, size_t remotePort,
                              const String& project, const String& from, const std::vector<String>& to,
                              const ResourceEstimatorPtr& resourceEstimator,
                              bool importTrace)
    : SubExecutionContext(parentContext), client(new ManagerNetworkClient(parentContext)),
      remoteHostName(remoteHostName), remotePort(remotePort),
      project(project), from(from), to(to), resourceEstimator(resourceEstimator),
      importTrace(importTrace), numWaitingTraces(0),
      asynchroneousWorkUnitPool(new AsynchroneousWorkUnitPool()),
      numSentWorkUnits(0), currentTo(0)
  {
    client->setCallback(this);
    client->startClient(remoteHostName, remotePort);
  }
  DistributedExecutionContext(ExecutionContext& parentContext,
                              const String& remoteHostName, size_t remotePort,
                              const String& project, const String& from, const String& to,
                              const ResourceEstimatorPtr& resourceEstimator,
                              bool importTrace)
  : SubExecutionContext(parentContext), client(new ManagerNetworkClient(parentContext)),
  remoteHostName(remoteHostName), remotePort(remotePort),
  project(project), from(from), to(std::vector<String>(1, to)), resourceEstimator(resourceEstimator),
  importTrace(importTrace), numWaitingTraces(0),
  asynchroneousWorkUnitPool(new AsynchroneousWorkUnitPool()),
  numSentWorkUnits(0), currentTo(0)
  {
    client->setCallback(this);
    client->startClient(remoteHostName, remotePort);
  }

  DistributedExecutionContext() {}

  virtual ~DistributedExecutionContext()
  {
    while (numWaitingTraces)
    { // In normal case, this message should not appear. It's an ultimate security before killing the connection.
      // If you are here, maybe you didn't correctly use the waitUntilAllWorkUnitsAreDone method !
      std::cout << "~DistributedExecutionContext - numWaitingTrace: " << numWaitingTraces << std::endl;
      juce::Thread::sleep(200);
    }
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

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, ExecutionContextCallbackPtr callback = ExecutionContextCallbackPtr(), bool pushIntoStack = true)
  {
    // TODO: pushIntoStack is not taken into account
    size_t index = numSentWorkUnits;
    if (sendWorkUnit(workUnit, asynchroneousWorkUnitPool, index))
      asynchroneousWorkUnitPool->workUnitSent(index, workUnit, callback);
    else
      warningCallback(T("DistributedExecutionContext::pushWorkUnit"), T("WorkUnit not sent !"));
  }
  
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
    {jassertfalse;}

  virtual void flushCallbacks()
    {asynchroneousWorkUnitPool->flushCallbacks();}

  virtual void waitUntilAllWorkUnitsAreDone(size_t timeOutInMilliseconds = 0)
    {waitUntilAllWorkUnitsAreDone(asynchroneousWorkUnitPool, timeOutInMilliseconds);}
    
  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack)
  {
    SingleWorkUnitPoolPtr pool = new SingleWorkUnitPool();
    sendWorkUnit(workUnit, pool, 0);
    waitUntilAllWorkUnitsAreDone(pool, 0);
    return pool->getResult();
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    CompositeWorkUnitPoolPtr pool = new CompositeWorkUnitPool(workUnits);
    const size_t n = workUnits->getNumWorkUnits();
    for (size_t i = 0; i < n; ++i)
      sendWorkUnit(workUnits->getWorkUnit(i), pool, i);
    waitUntilAllWorkUnitsAreDone(pool, 0);
    return pool->getResults();
  }

  /*
  ** ManagerNetworkClientCallback
  */
  virtual void workUnitAcknowledgementReceived(size_t sourceIdentifier, const String& uniqueIdentifier)
  {
    ScopedLock _(lock);
    jassert(sourceIdentifier < numSentWorkUnits);
    // Update identifier mapping
    workUnitIds[uniqueIdentifier] = sourceIdentifier;
  }

  virtual void workUnitResultReceived(const String& uniqueIdentifier, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(workUnitIds.count(uniqueIdentifier));
    const size_t internalId = workUnitIds[uniqueIdentifier];
    jassert(pools.count(internalId));
    std::pair<WorkUnitPoolPtr, size_t> poolAndId = pools[internalId];
    poolAndId.first->resultReceived(poolAndId.second, result, importTrace);
    if (!importTrace)
    {
      workUnitIds.erase(uniqueIdentifier);
      pools.erase(internalId);
    }
    else
    {
      client->askTrace(uniqueIdentifier);
      ++numWaitingTraces;
    }
  }

  virtual void traceReceived(const String& uniqueIdentifier, const ExecutionTracePtr& trace)
  {
    jassert(workUnitIds.count(uniqueIdentifier));
    const size_t internalId = workUnitIds[uniqueIdentifier];
    jassert(pools.count(internalId));
    std::pair<WorkUnitPoolPtr, size_t> poolAndId = pools[internalId];
    poolAndId.first->traceReceived(poolAndId.second, trace);
    workUnitIds.erase(uniqueIdentifier);
    pools.erase(internalId);
    --numWaitingTraces;
  }

  virtual void connectionMade()
  {
    ScopedLock _(lock);
    for (std::map<String, size_t>::iterator it = workUnitIds.begin(); it != workUnitIds.end(); ++it)
      client->sendVariable(new GetWorkUnitResultNetworkMessage(it->first));
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DistributedExecutionContextClass;

  ManagerNetworkClientPtr client;

  String remoteHostName;
  size_t remotePort;
  String project;
  String from;
  std::vector<String> to;
  ResourceEstimatorPtr resourceEstimator;
  bool importTrace;
  volatile size_t numWaitingTraces;

  AsynchroneousWorkUnitPoolPtr asynchroneousWorkUnitPool;

  bool sendWorkUnit(const WorkUnitPtr& workUnit, const WorkUnitPoolPtr& pool, size_t poolIndex)
  {
    size_t requiredCpus = resourceEstimator->getNumRequiredCpus(workUnit);
    size_t requiredMemory = resourceEstimator->getRequiredMemoryInMb(workUnit);
    size_t requiredTime = resourceEstimator->getRequiredTimeInHours(workUnit);

    ScopedLock _(lock);
    size_t internalId = numSentWorkUnits;
    pools[internalId] = std::make_pair(pool, poolIndex);

    if (!client->isConnected() && !client->startClient(remoteHostName, remotePort))
      return false;

    bool res = client->sendWorkUnit(internalId, workUnit, project, from, to[currentTo++], requiredCpus, requiredMemory, requiredTime);
    ++numSentWorkUnits;
    currentTo %= to.size();
    return res;
  }

  void waitUntilAllWorkUnitsAreDone(const WorkUnitPoolPtr& pool, size_t timeOutInMilliseconds)
  {
    AsynchroneousWorkUnitPoolPtr asynchroneousWorkUnitPool = pool.dynamicCast<AsynchroneousWorkUnitPool>();
    size_t counter = 0;
    while (!pool->areAllWorkUnitsDone())
    {
      ++counter;
      if (timeOutInMilliseconds && (1000 * counter > timeOutInMilliseconds))
        break;
      juce::Thread::sleep(1000);
      if (asynchroneousWorkUnitPool)
        asynchroneousWorkUnitPool->flushCallbacks();
    }

    /*if (timeOutInMilliseconds)
    {
      juce::Thread::sleep(timeOutInMilliseconds);
      if (asynchroneousWorkUnitPool)
        asynchroneousWorkUnitPool->flushCallbacks();
    }
    else
      while (!pool->areAllWorkUnitsDone())
      {
        juce::Thread::sleep(1000);
        if (asynchroneousWorkUnitPool)
          asynchroneousWorkUnitPool->flushCallbacks();
      }*/
  }

  CriticalSection lock;
  // There are three kind of identifiers:
  //  manager id -> global to the whole manager
  //  internal id -> global to this DistributedExecutionContext (=> number of sent work units)
  //  pool id -> local to one of the WorkUnitPools
  size_t numSentWorkUnits;
  std::map<String, size_t> workUnitIds; // manager ID -> internal ID
  std::map<size_t, std::pair<WorkUnitPoolPtr, size_t> > pools; // internal id -> (pool, pool id)
  size_t currentTo;
};

/*
** FixedResourceEstimator
*/
class FixedResourceEstimator : public ResourceEstimator
{
public:
  FixedResourceEstimator(size_t requiredCpus = 1, size_t requiredMemory = 1, size_t requiredTime = 1)
    : requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime) {}

  virtual size_t getNumRequiredCpus(const WorkUnitPtr& workUnit) const
    {return requiredCpus;}

  virtual size_t getRequiredMemoryInMb(const WorkUnitPtr& workUnit) const
    {return requiredMemory;}

  virtual size_t getRequiredTimeInHours(const WorkUnitPtr& workUnit) const
    {return requiredTime;}

protected:
  friend class FixedResourceEstimatorClass;

  size_t requiredCpus;
  size_t requiredMemory;
  size_t requiredTime;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_DISTRIBUTED_H_
