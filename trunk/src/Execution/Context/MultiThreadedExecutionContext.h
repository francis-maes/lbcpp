/*-----------------------------------------.---------------------------------.
| Filename: MultiThreadedExecutionContext.h| Multi-Threaded Execution        |
| Author  : Francis Maes                   | Context                         |
| Started : 24/11/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_MULTI_THREADED_H_
# define LBCPP_EXECUTION_CONTEXT_MULTI_THREADED_H_

# include <lbcpp/Execution/ExecutionContext.h>
# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Execution/WorkUnit.h>
# include "SubExecutionContext.h"
# include <list>

namespace lbcpp
{

class WaitingWorkUnitQueue : public Object
{
public:
  struct Entry
  {
    Entry(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int& counterToDecrementWhenDone)
      : workUnit(workUnit), stack(stack), counterToDecrementWhenDone(&counterToDecrementWhenDone) {}
    Entry() : counterToDecrementWhenDone(NULL) {}

    WorkUnitPtr workUnit;
    ExecutionStackPtr stack;
    int* counterToDecrementWhenDone;

    bool exists() const
      {return workUnit;}
  };

  void push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int& counterToDecrementWhenDone);
  void push(const CompositeWorkUnitPtr& workUnits, const ExecutionStackPtr& stack, int& numRemainingWorkUnitsCounter);

  Entry pop();

  lbcpp_UseDebuggingNewOperator

private:
  CriticalSection lock;
  typedef std::list<Entry> EntryList;
  std::vector<EntryList> entries;
};

typedef ReferenceCountedObjectPtr<WaitingWorkUnitQueue> WaitingWorkUnitQueuePtr;

/*
** WaitingWorkUnitQueue
*/
void WaitingWorkUnitQueue::push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int& counterToDecrementWhenDone)
{
  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  entries[priority].push_back(Entry(workUnit, stack, counterToDecrementWhenDone));
}

void WaitingWorkUnitQueue::push(const CompositeWorkUnitPtr& workUnits, const ExecutionStackPtr& stack, int& numRemainingWorkUnitsCounter)
{
  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  
  size_t n = workUnits->getNumWorkUnits();
  numRemainingWorkUnitsCounter = (int)n;
  for (size_t i = 0; i < n; ++i)
    entries[priority].push_back(Entry(workUnits->getWorkUnit(i), stack, numRemainingWorkUnitsCounter));
}

WaitingWorkUnitQueue::Entry WaitingWorkUnitQueue::pop()
{
  ScopedLock _(lock);
  for (int i = (int)entries.size() - 1; i >= 0; --i)
  {
    EntryList& l = entries[i];
    if (l.size())
    {
      Entry res = l.front();
      l.pop_front();
      return res;
    }
  }
  return Entry();
}

class WorkUnitThread;
extern ExecutionContextPtr threadOwnedExecutionContext(ExecutionContext& parentContext, WorkUnitThread* thread);

/*
** WorkUnitThread
*/
class WorkUnitThread : public Thread
{
public:
  WorkUnitThread(ExecutionContext& parentContext, size_t number, WaitingWorkUnitQueuePtr waitingQueue)
    : Thread(T("WorkUnitThread ") + String((int)number + 1)), waitingQueue(waitingQueue)
  {
    context = threadOwnedExecutionContext(parentContext, this);
  }

  virtual void run()
  {
    while (!threadShouldExit())
      processOneWorkUnit();
  }

  void workUntilWorkUnitsAreDone(int& counter)
  {
    while (!threadShouldExit() && counter)
    {
      bool ok = processOneWorkUnit();
      if (!ok)
        Thread::sleep(100); // waiting that the other threads are finished
    }
  }

  WaitingWorkUnitQueuePtr getWaitingQueue() const
    {return waitingQueue;}

  lbcpp_UseDebuggingNewOperator

private:
  ExecutionContextPtr context;
  WaitingWorkUnitQueuePtr waitingQueue;

  bool processOneWorkUnit()
  {
    WaitingWorkUnitQueue::Entry entry = waitingQueue->pop();
    if (!entry.exists())
    {
      Thread::sleep(10);
      return false;
    }

    ExecutionStackPtr previousStack = context->getStack();
    context->setStack(ExecutionStackPtr(new ExecutionStack(entry.stack)));
    context->run(entry.workUnit);
    juce::atomicDecrement(*entry.counterToDecrementWhenDone);
    context->setStack(previousStack);
    return true;
  }
};

/*
** WorkUnitThreadVector
*/
class WorkUnitThreadVector : public Object
{
public:
  WorkUnitThreadVector(size_t count)
    : threads(count, NULL) {}
  ~WorkUnitThreadVector()
    {stopAndDestroyAllThreads();}

  void startThread(size_t index, WorkUnitThread* newThread)
    {jassert(!threads[index]); threads[index] = newThread; newThread->startThread();}

  void stopAndDestroyAllThreads()
  {
    for (size_t i = 0; i < threads.size(); ++i)
      if (threads[i])
        threads[i]->signalThreadShouldExit();
    for (size_t i = 0; i < threads.size(); ++i)
      if (threads[i])
      {
        delete threads[i];
        threads[i] = NULL;
      }
  }

  size_t getNumThreads() const
    {return threads.size();}

  lbcpp_UseDebuggingNewOperator

private:
  std::vector<WorkUnitThread* > threads;
};

typedef ReferenceCountedObjectPtr<WorkUnitThreadVector> WorkUnitThreadVectorPtr;

/*
** ThreadOwnedExecutionContext
*/
class ThreadOwnedExecutionContext : public SubExecutionContext
{
public:
  ThreadOwnedExecutionContext(ExecutionContext& parentContext, WorkUnitThread* thread)
    : SubExecutionContext(parentContext), thread(thread) {}
  ThreadOwnedExecutionContext() : thread(NULL) {}

  virtual bool isMultiThread() const
    {return true;}

  virtual bool isCanceled() const
    {return thread->threadShouldExit();}

  virtual bool isPaused() const
    {return false;}
 
  virtual bool run(const WorkUnitPtr& workUnit)
    {return ExecutionContext::run(workUnit);}
  
  virtual bool run(const CompositeWorkUnitPtr& workUnits)
  {
    preExecutionCallback(stack, workUnits);
    stack->push(workUnits);
    int numRemainingWorkUnits;
    thread->getWaitingQueue()->push(workUnits, getStack()->cloneAndCast<ExecutionStack>(*this), numRemainingWorkUnits);
    thread->workUntilWorkUnitsAreDone(numRemainingWorkUnits);
    stack->pop();
    postExecutionCallback(stack, workUnits, true); // FIXME: result is not implemented
    return true;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  WorkUnitThread* thread;
};

ExecutionContextPtr threadOwnedExecutionContext(ExecutionContext& parentContext, WorkUnitThread* thread)
  {return ExecutionContextPtr(new ThreadOwnedExecutionContext(parentContext, thread));}

/*
** WorkUnitThreadPool
*/
class WorkUnitThreadPool : public Object
{
public:
  WorkUnitThreadPool(ExecutionContext& parentContext, size_t numThreads)
    : queue(new WaitingWorkUnitQueue()), threads(new WorkUnitThreadVector(numThreads))
  {
    for (size_t i = 0; i < numThreads; ++i)
      threads->startThread(i, new WorkUnitThread(parentContext, i, queue));
  }

  void waitUntilWorkUnitsAreDone(int& count)
  {
    while (count)
      Thread::sleep(10);
  }
 
  WaitingWorkUnitQueuePtr getWaitingQueue() const
    {return queue;}

  size_t getNumThreads() const
    {return threads->getNumThreads();}

  lbcpp_UseDebuggingNewOperator

private:
  WaitingWorkUnitQueuePtr queue;
  WorkUnitThreadVectorPtr threads;
};

typedef ReferenceCountedObjectPtr<WorkUnitThreadPool> WorkUnitThreadPoolPtr;

/*
** MultiThreadedExecutionContext
*/
class MultiThreadedExecutionContext : public ExecutionContext
{
public:
  MultiThreadedExecutionContext(size_t numThreads)
    {threadPool = WorkUnitThreadPoolPtr(new WorkUnitThreadPool(*this, numThreads));}
  MultiThreadedExecutionContext() {}

  virtual bool isMultiThread() const
    {return threadPool->getNumThreads() > 1;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit)
  {
    static int remainingWorkUnits;
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack, remainingWorkUnits);
  }

  virtual bool run(const WorkUnitPtr& workUnit)
  {
    int remainingWorkUnits = 1;
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack, remainingWorkUnits);
    threadPool->waitUntilWorkUnitsAreDone(remainingWorkUnits);
    return true;
  }

  virtual bool run(const CompositeWorkUnitPtr& workUnits)
  {
    preExecutionCallback(stack, workUnits);
    stack->push(workUnits);
    int numRemainingWorkUnits;
    threadPool->getWaitingQueue()->push(workUnits, getStack()->cloneAndCast<ExecutionStack>(*this), numRemainingWorkUnits);
    threadPool->waitUntilWorkUnitsAreDone(numRemainingWorkUnits);
    stack->pop();
    postExecutionCallback(stack, workUnits, true); // FIXME: result is not implemented
    return true;
  }

  lbcpp_UseDebuggingNewOperator

private:
  WorkUnitThreadPoolPtr threadPool;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_MULTI_THREADED_H_
