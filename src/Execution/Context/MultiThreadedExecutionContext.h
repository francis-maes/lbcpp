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
# include <list>

namespace lbcpp
{

class WaitingWorkUnitQueue : public Object
{
public:
  void push(WorkUnitPtr workUnit, int& counterToDecrementWhenDone, size_t priority);
  std::pair<WorkUnitPtr, int* > pop();

private:
  CriticalSection lock;
  std::vector< std::list< std::pair<WorkUnitPtr, int* > > > workUnits;
};

typedef ReferenceCountedObjectPtr<WaitingWorkUnitQueue> WaitingWorkUnitQueuePtr;

struct ThreadVector;

/*
** WaitingWorkUnitQueue
*/
void WaitingWorkUnitQueue::push(WorkUnitPtr workUnit, int& counterToDecrementWhenDone, size_t priority)
{
  ScopedLock _(lock);
  if (workUnits.size() <= priority)
    workUnits.resize(priority + 1);
  workUnits[priority].push_back(std::make_pair(workUnit, &counterToDecrementWhenDone));
}

std::pair<WorkUnitPtr, int* > WaitingWorkUnitQueue::pop()
{
  ScopedLock _(lock);
  for (int i = (int)workUnits.size() - 1; i >= 0; --i)
  {
    std::list< std::pair<WorkUnitPtr, int* > >& l = workUnits[i];
    if (l.size())
    {
      std::pair<WorkUnitPtr, int* > res = l.front();
      l.pop_front();
      return res;
    }
  }
  return std::make_pair(WorkUnitPtr(), (int* )0);
}

class WorkUnitThread;
extern ExecutionContextPtr threadOwnedExecutionContext(ExecutionContextPtr parentContext, WorkUnitThread* thread);

/*
** WorkUnitThread
*/
class WorkUnitThread : public Thread
{
public:
  WorkUnitThread(ExecutionContextPtr parentContext, size_t number, WaitingWorkUnitQueuePtr waitingQueue)
    : Thread(T("WorkUnitThread ") + String((int)number + 1)), parentContext(parentContext), waitingQueue(waitingQueue)
  {
    context = threadOwnedExecutionContext(parentContext, this);
  }

  bool processOneWorkUnit()
  {
    std::pair<WorkUnitPtr, int* > workUnit = waitingQueue->pop();
    if (!workUnit.first)
    {
      Thread::sleep(10);
      return false;
    }

    context->run(workUnit.first);
    jassert(workUnit.second);
    juce::atomicDecrement(*workUnit.second);
    return true;
  }

  void work()
  {
    while (!threadShouldExit())
      processOneWorkUnit();
  }

  void workUntilWorkUnitsAreDone(int& count)
  {
    while (!threadShouldExit() && count)
    {
      bool ok = processOneWorkUnit();
      jassert(ok);
    }
  }

  virtual void run()
    {work();}

  WaitingWorkUnitQueuePtr getWaitingQueue() const
    {return waitingQueue;}

private:
  ExecutionContextPtr parentContext;
  ExecutionContextPtr context;
  WaitingWorkUnitQueuePtr waitingQueue;
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

private:
  std::vector<WorkUnitThread* > threads;
};

typedef ReferenceCountedObjectPtr<WorkUnitThreadVector> WorkUnitThreadVectorPtr;

/*
** ThreadOwnedExecutionContext
*/
class ThreadOwnedExecutionContext : public DecoratorExecutionContext
{
public:
  ThreadOwnedExecutionContext(ExecutionContextPtr parentContext, WorkUnitThread* thread)
    : DecoratorExecutionContext(parentContext), thread(thread) {}
  ThreadOwnedExecutionContext() : thread(NULL) {}

  virtual bool isCanceled() const
    {return thread->threadShouldExit();}
 
  virtual bool run(const WorkUnitPtr& workUnit)
    {return ExecutionContext::run(workUnit);}

  virtual bool run(const std::vector<WorkUnitPtr>& workUnits)
  {
    WaitingWorkUnitQueuePtr queue = thread->getWaitingQueue();
    int numRemainingWorkUnits = workUnits.size();
    for (size_t i = 0; i < workUnits.size(); ++i)
      queue->push(workUnits[i], numRemainingWorkUnits, getStackDepth());
    thread->workUntilWorkUnitsAreDone(numRemainingWorkUnits);
    return true;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  WorkUnitThread* thread;
};

ExecutionContextPtr threadOwnedExecutionContext(ExecutionContextPtr parentContext, WorkUnitThread* thread)
  {return ExecutionContextPtr(new ThreadOwnedExecutionContext(parentContext, thread));}

/*
** WorkUnitThreadPool
*/
class WorkUnitThreadPool : public Object
{
public:
  WorkUnitThreadPool(ExecutionContextPtr parentContext, size_t numThreads)
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

private:
  WaitingWorkUnitQueuePtr queue;
  WorkUnitThreadVectorPtr threads;
};

typedef ReferenceCountedObjectPtr<WorkUnitThreadPool> WorkUnitThreadPoolPtr;

///////////////////////////////////////////
//////////////////////////////////////////
class MultiThreadedExecutionContext : public ExecutionContext
{
public:
  MultiThreadedExecutionContext(size_t numThreads)
  {
    threadPool = WorkUnitThreadPoolPtr(new WorkUnitThreadPool(refCountedPointerFromThis(this), numThreads));
  }
  MultiThreadedExecutionContext() {}

  virtual bool isMultiThread() const
    {return true;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual bool run(const WorkUnitPtr& workUnit)
  {
    int remainingWorkUnits = 1;
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    queue->push(workUnit, remainingWorkUnits, 0);
    threadPool->waitUntilWorkUnitsAreDone(remainingWorkUnits);
    return true;
  }

  virtual bool run(const std::vector<WorkUnitPtr>& workUnits)
  {
    int numRemainingWorkUnits = workUnits.size();
    WaitingWorkUnitQueuePtr queue = threadPool->getWaitingQueue();
    for (size_t i = 0; i < workUnits.size(); ++i)
      queue->push(workUnits[i], numRemainingWorkUnits, 0);
    threadPool->waitUntilWorkUnitsAreDone(numRemainingWorkUnits);
    return true;
  }

private:
  WorkUnitThreadPoolPtr threadPool;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_MULTI_THREADED_H_
