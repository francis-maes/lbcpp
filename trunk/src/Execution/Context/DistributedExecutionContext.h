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

  void appendWorkUnit(size_t internalId, const WorkUnitPtr& workUnit)
  {
    ScopedLock _(lock);
    indices[internalId] = workUnits.size();
    workUnits.push_back(workUnit);
    results.push_back(String::empty);
  }

  void setResult(size_t internalId, const Variable& result)
  {
    ScopedLock _(lock);
    jassert(indices.count(internalId) && indices[internalId] < results.size());
    results[indices[internalId]] = result;
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
};

typedef ReferenceCountedObjectPtr<WorkUnitPool> WorkUnitPoolPtr;

class XxxDistributedExecutionContextNetworkClient : public XxxNetworkClient
{
public:
  XxxDistributedExecutionContextNetworkClient(ExecutionContext& context)
    : XxxNetworkClient(context), numSentWorkUnit(0)
    {}

  virtual void variableReceived(const Variable& variable)
  {
    ScopedLock _(lock);
    
    if (!variable.isObject())
    {
      context.warningCallback(T("XxxDistributedExecutionContextNetworkClient::variableReceived")
                              , T("The message is not an Object ! The message is ") + variable.toString().quoted());
      return;
    }

    const ObjectPtr obj = variable.getObject();
    if (!obj)
    {
      context.warningCallback(T("XxxDistributedExecutionContextNetworkClient::variableReceived")
                              , T("NULL Object"));
      return;
    }

    const ClassPtr objClass = obj->getClass();
    if (objClass == workUnitAcknowledgementNetworkMessageClass)
    {
      WorkUnitAcknowledgementNetworkMessagePtr ack = obj.staticCast<WorkUnitAcknowledgementNetworkMessage>();
      jassert(ack->getSourceIdentifier() < numSentWorkUnit);
      // Update identifier mapping
      workUnitIds[ack->getUniqueIdentifier()] = ack->getSourceIdentifier();
    }
    else if (objClass == workUnitResultNetworkMessageClass)
    {
      WorkUnitResultNetworkMessagePtr res = obj.staticCast<WorkUnitResultNetworkMessage>();
      jassert(workUnitIds.count(res->getUniqueIdentifier()));
      const size_t internalId = workUnitIds[res->getUniqueIdentifier()];
      jassert(pools.count(internalId));
      WorkUnitPoolPtr pool = pools[internalId];
      pool->setResult(internalId, res->getResult(context));

      workUnitIds.erase(res->getUniqueIdentifier());
      pools.erase(internalId);
    }
    else
    {
      context.warningCallback(T("XxxDistributedExecutionContextNetworkClient::variableReceived")
                              , T("Unknwon object of type: ") + objClass->toString());
    }
  }

  void sendWorkUnit(const WorkUnitPtr& workUnit, const WorkUnitPoolPtr& pool)
  {
    ScopedLock _(lock);
    pool->appendWorkUnit(numSentWorkUnit, workUnit);
    pools[numSentWorkUnit] = pool;
    sendVariable(new WorkUnitNetworkMessage(context, numSentWorkUnit, workUnit));
    ++numSentWorkUnit;
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

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
  {
    // FIXME: No mechanisme to retreive result
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
