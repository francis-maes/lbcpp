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
    Entry(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, bool pushIntoStack, int* counterToDecrementWhenDone, Variable* result)
      : workUnit(workUnit), stack(stack), pushIntoStack(pushIntoStack), counterToDecrementWhenDone(counterToDecrementWhenDone), result(result) {}
    Entry() : counterToDecrementWhenDone(NULL) {}

    WorkUnitPtr workUnit;
    ExecutionStackPtr stack;
    bool pushIntoStack;
    int* counterToDecrementWhenDone;
    Variable* result;

    bool exists() const
      {return workUnit;}
  };

  void push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int* counterToDecrementWhenDone = NULL, Variable* result = NULL);
  void push(const CompositeWorkUnitPtr& workUnits, const ExecutionStackPtr& stack, int* numRemainingWorkUnitsCounter = NULL, Variable* result = NULL);

  Entry pop();

  bool isEmpty() const;

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
void WaitingWorkUnitQueue::push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int* counterToDecrementWhenDone, Variable* result)
{
  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  entries[priority].push_back(Entry(workUnit, stack, true, counterToDecrementWhenDone, result));
  //std::cout << "WaitingWorkUnitQueue::push - ClassName : " << workUnit->getClassName() << " - Description : " << workUnit->toString() << std::endl;
}

void WaitingWorkUnitQueue::push(const CompositeWorkUnitPtr& workUnits, const ExecutionStackPtr& stack, int* numRemainingWorkUnitsCounter, Variable* result)
{
  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  
  size_t n = workUnits->getNumWorkUnits();
  *numRemainingWorkUnitsCounter = (int)n;
  //*result = true;
  for (size_t i = 0; i < n; ++i)
    entries[priority].push_back(Entry(workUnits->getWorkUnit(i), stack, workUnits->hasPushChildrenIntoStackFlag(), numRemainingWorkUnitsCounter, NULL));
  //std::cout << "WaitingWorkUnitQueue::push - ClassName : " << workUnits->getClassName() << " - Description : " << workUnits->toString() << " - Composite : " << n << std::endl;
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

bool WaitingWorkUnitQueue::isEmpty() const
{
  ScopedLock _(lock); 
  for (int i = (int)entries.size() - 1; i >= 0; --i)
    if (entries[i].size())
      return false;
  return true;
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

  void workUntilWorkUnitsAreDone(const CompositeWorkUnitPtr& workUnits, int& counter)
  {
    size_t n = workUnits->getNumWorkUnits();
    ProgressionStatePtr progression(new ProgressionState(0.0, (double)n, workUnits->getProgressionUnit()));
    while (!threadShouldExit() && counter)
    {
      progression->setValue((double)(n - counter));
      context->progressCallback(progression);
      bool ok = processOneWorkUnit();
      if (!ok)
        Thread::sleep(100); // waiting that the other threads are finished
    }
    progression->setValue((double)(n - counter));
    context->progressCallback(progression);
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

    // set new stack into context
    ExecutionStackPtr previousStack = context->getStack();
    context->setStack(ExecutionStackPtr(new ExecutionStack(entry.stack)));

    // thread begin callback
    context->threadBeginCallback(context->getStack());

    // execute work unit
    Variable result = context->run(entry.workUnit, entry.pushIntoStack);

    // update result and counterToDecrement
    if (entry.result)
      *entry.result = result;
    if (entry.counterToDecrementWhenDone)
      juce::atomicDecrement(*entry.counterToDecrementWhenDone);

    // thread end callback and restore previous stack
    context->threadEndCallback(context->getStack());
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
    threads.clear();
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
 
  virtual Variable run(const WorkUnitPtr& workUnit)
    {return ExecutionContext::run(workUnit);}
  
  static void startParallelRun(ExecutionContext& context, const CompositeWorkUnitPtr& workUnits, WaitingWorkUnitQueuePtr waitingQueue, int& numRemainingWorkUnits, Variable& result)
  {
    waitingQueue->push(workUnits, context.getStack()->cloneAndCast<ExecutionStack>(context), &numRemainingWorkUnits, &result);
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    CompositeWorkUnitPtr compactedWorkUnits = ThreadOwnedExecutionContext::compactWorkUnitIfNecessary(workUnits, 24);
    Variable result;
    int numRemainingWorkUnits;
    if (pushIntoStack)
      enterScope(workUnits);
    startParallelRun(*this, workUnits, thread->getWaitingQueue(), numRemainingWorkUnits, result);
    thread->workUntilWorkUnitsAreDone(workUnits, numRemainingWorkUnits);
    if (pushIntoStack)
      leaveScope(result);
    return result;
  }

  static CompositeWorkUnitPtr compactWorkUnitIfNecessary(const CompositeWorkUnitPtr& workUnits, size_t numThreads)
  {
    size_t n = workUnits->getNumWorkUnits();
    if (n < numThreads * 25)
      return workUnits;

    CompositeWorkUnitPtr res = new CompositeWorkUnit(workUnits->getName() + T(" (compacted)"), numThreads);
    res->setProgressionUnit(workUnits->getProgressionUnit());
    res->setPushChildrenIntoStackFlag(false);

    size_t numWorkUnitByComposite = n / numThreads;
    size_t remainingWorkUnit = n % numThreads;
    std::vector<CompositeWorkUnitPtr> compactedWorkUnits(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
    {
      compactedWorkUnits[i] = new CompositeWorkUnit(workUnits->getName() + T("(compacted ") + String((int)i) + T(")"),
                                                    numWorkUnitByComposite + (i < remainingWorkUnit ? 1 : 0));
      compactedWorkUnits[i]->setPushChildrenIntoStackFlag(workUnits->hasPushChildrenIntoStackFlag());
      res->setWorkUnit(i, compactedWorkUnits[i]);
    }

    for (size_t i = 0; i < n; ++i)
      compactedWorkUnits[i % numThreads]->setWorkUnit(i / numThreads, workUnits->getWorkUnit(i));
    return res;
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
 
  void waitUntilAllWorkUnitsAreDone()
  {
    while (!queue->isEmpty())
      Thread::sleep(10);
  }

  void stopAndDestroyAllThreads()
    {threads->stopAndDestroyAllThreads();}

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

  virtual ~MultiThreadedExecutionContext()
  {
    if (threadPool)
      threadPool->stopAndDestroyAllThreads();
  }

  virtual String toString() const
    {return T("MultiThreaded(") + String((int)threadPool->getNumThreads()) + T(")");}

  virtual bool isMultiThread() const
    {return threadPool->getNumThreads() > 1;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit)
  {
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack);
  }

  virtual void waitUntilAllWorkUnitsAreDone()
    {threadPool->waitUntilAllWorkUnitsAreDone();}
    
  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack)
  {
    //std::cout << "MultiThreadedExecutionContext::run - WorkUnit - Description : " << workUnit->getDescription() << std::endl;
    int remainingWorkUnits = 1;
    Variable result;
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack, &remainingWorkUnits, &result);
    threadPool->waitUntilWorkUnitsAreDone(remainingWorkUnits);
    return result;
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    CompositeWorkUnitPtr compactedWorkUnits = ThreadOwnedExecutionContext::compactWorkUnitIfNecessary(workUnits, threadPool->getNumThreads());
    //std::cout << "MultiThreadedExecutionContext::run - CompositeWorkUnit - Description : " << workUnits->getDescription() << std::endl;
    int numRemainingWorkUnits;
    Variable result;
    if (pushIntoStack)
      enterScope(workUnits->getName(), workUnits);
    ThreadOwnedExecutionContext::startParallelRun(*this, workUnits, threadPool->getWaitingQueue(), numRemainingWorkUnits, result);
    threadPool->waitUntilWorkUnitsAreDone(numRemainingWorkUnits);
    if (pushIntoStack)
      leaveScope(result);
    return result;
  }

  lbcpp_UseDebuggingNewOperator

private:
  WorkUnitThreadPoolPtr threadPool;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_MULTI_THREADED_H_
