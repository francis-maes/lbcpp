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

class GroupedCompositeWorkUnit : public CompositeWorkUnit
{
public:
  class WorkUnitBatch : public WorkUnit
  {
  public:
    WorkUnitBatch(const CompositeWorkUnitPtr& workUnits, size_t begin, size_t end, VectorPtr results)
      : workUnits(workUnits), begin(begin), end(end),results(results)
    {
    }

    virtual Variable run(ExecutionContext& context)
    {
      jassert(results);

      bool pushIntoStack = workUnits->hasPushChildrenIntoStackFlag();
      for (size_t i = begin; i < end; ++i)
      {
        Variable res = context.run(workUnits->getWorkUnit(i), pushIntoStack);
        results->setElement(i, res);
      }
      return true;
    }
    
  private:
    CompositeWorkUnitPtr workUnits;
    size_t begin, end;
    VectorPtr results;
  };

  GroupedCompositeWorkUnit(const CompositeWorkUnitPtr& workUnits, size_t numBatches, VectorPtr results)
    : CompositeWorkUnit(workUnits->toString(), numBatches), workUnits(workUnits)
  {
    size_t n = workUnits->getNumWorkUnits();
    meanSizePerBatch = n / (double)numBatches;
    for (size_t i = 0; i < numBatches; ++i)
    {
      size_t begin = i * n / numBatches;
      size_t end = (i + 1) * n / numBatches;
      jassert(end <= n);
      setWorkUnit(i, new WorkUnitBatch(workUnits, begin, end, results));
    }
    setProgressionUnit(workUnits->getProgressionUnit());
    setPushChildrenIntoStackFlag(false);
  }

  virtual ProgressionStatePtr getProgression(size_t numWorkUnitsDone) const
    {return new ProgressionState((double)(int)(numWorkUnitsDone * meanSizePerBatch + 0.5), (double)workUnits->getNumWorkUnits(), progressionUnit);}

private:
  CompositeWorkUnitPtr workUnits;
  double meanSizePerBatch;
};

class WaitingWorkUnitQueue : public Object
{
public:
  WaitingWorkUnitQueue(size_t numThreads = 0)
    : numThreads(numThreads) {}

  struct Entry
  {
    Entry(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, bool pushIntoStack, int* counterToDecrementWhenDone, Variable* result, const ExecutionContextCallbackPtr& callback = ExecutionContextCallbackPtr())
      : workUnit(workUnit), stack(stack), pushIntoStack(pushIntoStack), counterToDecrementWhenDone(counterToDecrementWhenDone), result(result), callback(callback) {}
    Entry() : counterToDecrementWhenDone(NULL) {}

    WorkUnitPtr workUnit;
    ExecutionStackPtr stack;
    bool pushIntoStack;
    int* counterToDecrementWhenDone;
    Variable* result;
    ExecutionContextCallbackPtr callback;

    bool exists() const
      {return workUnit;}
  };

  void push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true, Variable* result = NULL);
  void push(const CompositeWorkUnitPtr& workUnits, const ExecutionStackPtr& stack, int* numRemainingWorkUnitsCounter = NULL, VariableVectorPtr results = VariableVectorPtr());
  void push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, ExecutionContextCallbackPtr callback, bool pushIntoStack = true);

  Entry pop();

  bool isEmpty() const;

  size_t getNumThreads() const
    {return numThreads;}

  void pushCallbackCall(ExecutionContextCallbackPtr callback, const WorkUnitPtr& workUnit, const Variable& result)
  {
    CallbackInfo info;
    info.callback = callback;
    info.workUnit = workUnit;
    info.result = result;
    ScopedLock _(callbacksLock);
    callbacks.push_back(info);
  }

  void flushCallbacks()
  {
    std::list<CallbackInfo> callbacks;
    {
      ScopedLock _(callbacksLock);
      this->callbacks.swap(callbacks);
    }
    for (std::list<CallbackInfo>::iterator it = callbacks.begin(); it != callbacks.end(); ++it)
      it->callback->workUnitFinished(it->workUnit, it->result);
  }

  lbcpp_UseDebuggingNewOperator

private:
  CriticalSection lock;
  typedef std::list<Entry> EntryList;
  std::vector<EntryList> entries;
  size_t numThreads;

  CriticalSection callbacksLock;
  struct CallbackInfo
  {
    ExecutionContextCallbackPtr callback;
    WorkUnitPtr workUnit;
    Variable result;
  };
  std::list<CallbackInfo> callbacks;
};

typedef ReferenceCountedObjectPtr<WaitingWorkUnitQueue> WaitingWorkUnitQueuePtr;

/*
** WaitingWorkUnitQueue
*/
void WaitingWorkUnitQueue::push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, int* counterToDecrementWhenDone, bool pushIntoStack, Variable* result)
{
  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  entries[priority].push_back(Entry(workUnit, stack->cloneAndCast<ExecutionStack>(), pushIntoStack, counterToDecrementWhenDone, result));
}

void WaitingWorkUnitQueue::push(const CompositeWorkUnitPtr& workUnits, const ExecutionStackPtr& s, int* numRemainingWorkUnitsCounter, VariableVectorPtr results)
{
  ExecutionStackPtr stack = s->cloneAndCast<ExecutionStack>();

  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  
  size_t n = workUnits->getNumWorkUnits();

  *numRemainingWorkUnitsCounter = (int)n;
  for (size_t i = 0; i < n; ++i)
    entries[priority].push_back(Entry(workUnits->getWorkUnit(i), stack, workUnits->hasPushChildrenIntoStackFlag(), numRemainingWorkUnitsCounter, results ? results->getPointerElement(i) : NULL));
}

void WaitingWorkUnitQueue::push(const WorkUnitPtr& workUnit, const ExecutionStackPtr& stack, ExecutionContextCallbackPtr callback, bool pushIntoStack)
{
  ScopedLock _(lock);
  size_t priority = stack->getDepth();
  if (entries.size() <= priority)
    entries.resize(priority + 1);
  entries[priority].push_back(Entry(workUnit, stack->cloneAndCast<ExecutionStack>(), pushIntoStack, NULL, NULL, callback));  
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
    context->setProjectDirectory(parentContext.getProjectDirectory());
  }
  virtual ~WorkUnitThread()
  {
    stopThread(100);
    context = ExecutionContextPtr();
    waitingQueue = WaitingWorkUnitQueuePtr();
  }

  virtual void run()
  {
    while (!threadShouldExit())
      processOneWorkUnit();
  }

  void workUntilWorkUnitsAreDone(const CompositeWorkUnitPtr& workUnits, int& counter)
  {
    size_t n = workUnits->getNumWorkUnits();
    while (!threadShouldExit() && counter)
    {
      context->progressCallback(workUnits->getProgression(n - counter));
      bool ok = processOneWorkUnit();
      if (!ok)
        Thread::sleep(100); // waiting that the other threads are finished
    }
    context->progressCallback(workUnits->getProgression(n));
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
    ExecutionStackPtr entryStack = entry.stack->cloneAndCast<ExecutionStack>();
    context->setStack(entryStack);

    // thread begin callback
    context->threadBeginCallback(entryStack);

    // execute work unit
    Variable result = context->run(entry.workUnit, entry.pushIntoStack);

    // update result and counterToDecrement
    if (entry.result)
      *entry.result = result;
    if (entry.counterToDecrementWhenDone)
      juce::atomicDecrement(*entry.counterToDecrementWhenDone);
    if (entry.callback)
      waitingQueue->pushCallbackCall(entry.callback, entry.workUnit, result);

    // thread end callback and restore previous stack
    context->threadEndCallback(entryStack);
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

  virtual ~ThreadOwnedExecutionContext() {}

  virtual bool isMultiThread() const
    {return true;}

  virtual bool isCanceled() const
    {return thread->threadShouldExit();}

  virtual bool isPaused() const
    {return false;}
 
  virtual Variable run(const WorkUnitPtr& workUnit)
    {return ExecutionContext::run(workUnit);}
  
  static void startParallelRun(ExecutionContext& context, CompositeWorkUnitPtr& workUnits, WaitingWorkUnitQueuePtr waitingQueue, int& numRemainingWorkUnits, Variable& result)
  {
    size_t numWorkUnits = workUnits->getNumWorkUnits();
    size_t numThreads = waitingQueue->getNumThreads();
    size_t maxInParallel = numThreads * 5;

    VectorPtr results = variableVector(numWorkUnits);
    result = Variable(results);
    if (numWorkUnits > maxInParallel)
    {
      // in this case, the GroupedCompositeWorkUnit is responsible for directly filling the results vector
      workUnits = new GroupedCompositeWorkUnit(workUnits, maxInParallel, results);
      waitingQueue->push(workUnits, context.getStack(), &numRemainingWorkUnits);
    }
    else
    {
      // here, it is the execution context that is responsible for filling the results vector
      waitingQueue->push(workUnits, context.getStack(), &numRemainingWorkUnits, results);
    }
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    Variable result;
    int numRemainingWorkUnits;
    if (pushIntoStack)
      enterScope(workUnits);
    CompositeWorkUnitPtr wus = workUnits;
    startParallelRun(*this, wus, thread->getWaitingQueue(), numRemainingWorkUnits, result);
    thread->workUntilWorkUnitsAreDone(wus, numRemainingWorkUnits);
    if (pushIntoStack)
      leaveScope(result);
    return result;
  }

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, ExecutionContextCallbackPtr callback = NULL, bool pushIntoStack = true)
  {
    WaitingWorkUnitQueuePtr queue = thread->getWaitingQueue();
    queue->push(workUnit, stack, callback, pushIntoStack);
  }

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
  {
    WaitingWorkUnitQueuePtr queue = thread->getWaitingQueue();
    queue->push(workUnit, stack, counterToDecrementWhenDone, pushIntoStack);
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
    : queue(new WaitingWorkUnitQueue(numThreads)), threads(new WorkUnitThreadVector(numThreads))
  {
    for (size_t i = 0; i < numThreads; ++i)
      threads->startThread(i, new WorkUnitThread(parentContext, i, queue));
  }

  void waitUntilWorkUnitsAreDone(int& count)
  {
    while (count)
      Thread::sleep(10);
  }
 
  void waitUntilAllWorkUnitsAreDone(size_t timeOutInMilliseconds)
  {
    size_t time = 0;
    while (!queue->isEmpty())
    {
      Thread::sleep(10);
      queue->flushCallbacks();
      time += 10;
      if (timeOutInMilliseconds && time >= timeOutInMilliseconds)
        break;
    }
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
  MultiThreadedExecutionContext(size_t numThreads, const File& projectDirectory)
    : ExecutionContext(projectDirectory)
    {
      threadPool = WorkUnitThreadPoolPtr(new WorkUnitThreadPool(*this, numThreads));
    }
  MultiThreadedExecutionContext() {}

  virtual ~MultiThreadedExecutionContext()
  {
    if (threadPool)
      threadPool->stopAndDestroyAllThreads();
    threadPool = WorkUnitThreadPoolPtr();
  }

  virtual String toString() const
    {return T("MultiThreaded(") + String((int)threadPool->getNumThreads()) + T(")");}

  virtual bool isMultiThread() const
    {return threadPool->getNumThreads() > 1;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, ExecutionContextCallbackPtr callback = NULL, bool pushIntoStack = true)
  {
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack, callback, pushIntoStack);
  }

  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
  {
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack, counterToDecrementWhenDone, pushIntoStack);
  }

  virtual void waitUntilAllWorkUnitsAreDone(size_t timeOutInMilliseconds)
    {threadPool->waitUntilAllWorkUnitsAreDone(timeOutInMilliseconds);}
   
  virtual void flushCallbacks()
    {threadPool->getWaitingQueue()->flushCallbacks();}
    

  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack)
  {
    int remainingWorkUnits = 1;
    Variable result;
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, stack, &remainingWorkUnits, pushIntoStack, &result);
    threadPool->waitUntilWorkUnitsAreDone(remainingWorkUnits);
    return result;
  }

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack)
  {
    int numRemainingWorkUnits;
    Variable result;
    if (pushIntoStack)
      enterScope(workUnits->getName(), workUnits);
    CompositeWorkUnitPtr wus = workUnits;
    ThreadOwnedExecutionContext::startParallelRun(*this, wus, threadPool->getWaitingQueue(), numRemainingWorkUnits, result);
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
