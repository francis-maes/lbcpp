/*-----------------------------------------.---------------------------------.
| Filename: ThreadPool.cpp                 | Thread Pool                     |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 18:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/ThreadPool.h>
#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/Data/Cache.h>
using namespace lbcpp;

namespace lbcpp
{

/*
** MultipleWaitableEvent
*/
class MultipleWaitableEvent : public Object
{
public:
  MultipleWaitableEvent() : count(0) {}

  bool wait(size_t count, int timeOutMilliseconds = -1)
  {
    while (true)
    {
      {
        ScopedLock _(countLock);
        if (this->count >= count)
          return true;
      }
      if (!event.wait(timeOutMilliseconds))
        return false;
    }
  }

  void signal()
  {
    {
      ScopedLock _(countLock);
      ++count;
    }
    event.signal();
  }

private:
  juce::WaitableEvent event;
  CriticalSection countLock;
  size_t count;
};

struct SignalingWorkUnit : public DecoratorWorkUnit
{
  SignalingWorkUnit(WorkUnitPtr workUnit, MultipleWaitableEventPtr event)
    : DecoratorWorkUnit(workUnit), event(event) {}

  MultipleWaitableEventPtr event;

  virtual bool run(ExecutionContext& context)
  {
    bool res = DecoratorWorkUnit::run(context);
    event->signal();
    return res;
  }
};

}; /* namespace lbcpp */

/*
** ThreadPool
*/
ThreadPool::ThreadPool(size_t numCpus, bool verbose)
  : numCpus(numCpus), verbose(verbose), timingsCache(new AverageValuesCache()) {}

ThreadPool::~ThreadPool()
{
  for (size_t i = 0; i < threads.size(); ++i)
    threads[i]->signalThreadShouldExit();
  for (size_t i = 0; i < threads.size(); ++i)
    delete threads[i];
}

void ThreadPool::addWorkUnit(WorkUnitPtr workUnit, size_t priority, MultipleWaitableEventPtr event)
{
  ScopedLock _(waitingWorkUnitsLock);
  if (waitingWorkUnits.size() <= priority)
    waitingWorkUnits.resize(priority + 1);
  waitingWorkUnits[priority].push_back(new SignalingWorkUnit(workUnit, event));
}

void ThreadPool::addWorkUnits(const std::vector<WorkUnitPtr>& workUnits, size_t priority, MultipleWaitableEventPtr event)
{
  ScopedLock _(waitingWorkUnitsLock);
  if (waitingWorkUnits.size() <= priority)
    waitingWorkUnits.resize(priority + 1);

  std::list<WorkUnitPtr>& waiting = waitingWorkUnits[priority];
  for (size_t i = 0; i < workUnits.size(); ++i)
    waiting.push_back(new SignalingWorkUnit(workUnits[i], event));
}

WorkUnitPtr ThreadPool::popWorkUnit()
{
  ScopedLock _(waitingWorkUnitsLock);
  for (int i = (int)waitingWorkUnits.size() - 1; i >= 0; --i)
  {
    std::list<WorkUnitPtr>& workUnits = waitingWorkUnits[i];
    if (workUnits.size())
    {
      WorkUnitPtr res = workUnits.front();
      workUnits.pop_front();
      return res;
    }
  }
  return NULL;
}


size_t ThreadPool::getNumWaitingThreads() const
  {ScopedLock _(threadsLock); return waitingThreads.size();}

size_t ThreadPool::getNumRunningThreads() const
  {ScopedLock _(threadsLock); jassert(threads.size() >= waitingThreads.size()); return threads.size() - waitingThreads.size();}

size_t ThreadPool::getNumThreads() const
  {ScopedLock _(threadsLock); return threads.size();}

void ThreadPool::update(ExecutionContext& context)
{
  ScopedLock _(threadsLock);
  for (size_t i = 0; i < threads.size(); )
  {
    if (!threads[i]->isThreadRunning())
    {
      delete threads[i];
      threads.erase(threads.begin() + i);
    }
    else
      ++i;
  }
  while (getNumRunningThreads() < numCpus)
  {
    WorkUnitPtr workUnit = popWorkUnit();
    if (workUnit)
    {
      if (verbose)
        context.informationCallback(T("Start Work Unit: ") + workUnit->getName());
      startThreadForWorkUnit(workUnit);
    }
    else
      break;
  }

  if (verbose)
  {
    static int counter = 0;
    if (++counter % 1000 == 0)
    {
      context.informationCallback(T("\n==============="));
      writeCurrentState(std::cout);
      context.informationCallback(String::empty);
    }
  }
}

void ThreadPool::addWorkUnitAndWaitExecution(WorkUnitPtr workUnit, size_t priority, bool callUpdateWhileWaiting)
{
  MultipleWaitableEventPtr event = new MultipleWaitableEvent();
  addWorkUnit(workUnit, priority, event);
  if (callUpdateWhileWaiting)
    while (!event->wait(1, 1))
      update(*silentExecutionContext);
  else
    event->wait(1);
}

void ThreadPool::addWorkUnitsAndWaitExecution(const std::vector<WorkUnitPtr>& workUnits, size_t priority, bool callUpdateWhileWaiting)
{
  Thread* currentThread = Thread::getCurrentThread();
  MultipleWaitableEventPtr event = new MultipleWaitableEvent();
  {
    ScopedLock _(threadsLock);
    addWorkUnits(workUnits, priority, event);
    if (currentThread)
    {
      for (size_t i = 0; i < threads.size(); ++i)
        if (threads[i] == currentThread)
        {
          waitingThreads.insert(currentThread);
          break;
        }
    }
  }

  if (callUpdateWhileWaiting)
    while (!event->wait(workUnits.size(), 1))
      update(*silentExecutionContext);
  else
    event->wait(workUnits.size());
  
  {
    ScopedLock _(threadsLock);
    if (currentThread)
      waitingThreads.erase(currentThread);
    update(*silentExecutionContext);
    if (getNumRunningThreads() == 0 && getNumWaitingThreads() > 1)
    {
      std::cerr << std::endl << "Fatal Error: Not any running thread, Probable Dead Lock!!!" << std::endl;
      writeCurrentState(std::cerr);
      static int counter = 0;
      if (++counter == 100)
        exit(1);
    }
  }
}

bool ThreadPool::isThreadWaiting(juce::Thread* thread) const
{
  ScopedLock _(threadsLock);
  return waitingThreads.find(thread) != waitingThreads.end();
}

#include "Context/ThreadOwnedExecutionContext.h"

void ThreadPool::startThreadForWorkUnit(WorkUnitPtr workUnit)
{
  ScopedLock _(threadsLock);
  Thread* res = new ThreadOwnedExecutionContext(singleThreadedExecutionContext(), workUnit);
  res->startThread();
  threads.push_back(res);
}

void ThreadPool::writeCurrentState(std::ostream& ostr)
{
  ScopedLock _1(threadsLock);
  ScopedLock _2(waitingWorkUnitsLock);

  size_t numWaitingWorkUnits = 0;
  for (size_t i = 0; i < waitingWorkUnits.size(); ++i)
    numWaitingWorkUnits += waitingWorkUnits[i].size();

  ostr << numCpus << " cpus, " << getNumWaitingThreads() << " paused threads, "
      << getNumRunningThreads() << " running threads, "
      << numWaitingWorkUnits << " waiting work units" << std::endl;

  for (size_t i = 0; i < threads.size(); ++i)
  {
    ThreadOwnedExecutionContext* thread = dynamic_cast<ThreadOwnedExecutionContext* >(threads[i]);
    jassert(thread);
    WorkUnitPtr workUnit = thread->getWorkUnit();
    ostr << (isThreadWaiting(thread) ? "W" : "A") << " " << workUnit->getName() << std::endl << std::endl;
  }
  if (numWaitingWorkUnits)
  {
    ostr << "- Queue - " << std::endl;
    for (int i = (int)waitingWorkUnits.size() - 1; i >= 0; --i)
    {
      const std::list<WorkUnitPtr>& workUnits = waitingWorkUnits[i];
      for (std::list<WorkUnitPtr>::const_iterator it = workUnits.begin(); it != workUnits.end(); ++it)
        ostr << "[" << i << "] " << (*it)->getName() << std::endl;
    }
  }
}
