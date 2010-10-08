/*-----------------------------------------.---------------------------------.
| Filename: ThreadPool.cpp                 | Thread Pool                     |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 18:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ThreadPool.h>
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

/*
** RunJobThread
*/
class RunJobThread : public Thread
{
public:
  RunJobThread(JobPtr job)
    : Thread(job->getName()), job(job) {}

  void signalShouldExit()
  {
    job->signalJobShouldExit();
    signalThreadShouldExit();
  }

  virtual void run()
  {
    juce::SystemStats::initialiseStats();
    String failureReason;
    job->runJob(failureReason);
  }

  JobPtr getJob() const
    {return job;}

  juce_UseDebuggingNewOperator

private:
  JobPtr job;
};

class SignalingJob : public Job
{
public:
  SignalingJob(JobPtr job, MultipleWaitableEventPtr event)
    : Job(job->getName()), job(job), event(event) {}

  virtual String getCurrentStatus() const
    {return job->getCurrentStatus();}

  virtual bool runJob(String& failureReason)
  {
    bool res = job->runJob(failureReason);
    event->signal();
    return res;
  }

protected:
  JobPtr job;
  MultipleWaitableEventPtr event;
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
    ((RunJobThread* )threads[i])->signalShouldExit();
  for (size_t i = 0; i < threads.size(); ++i)
    delete threads[i];
}

void ThreadPool::addJob(JobPtr job, size_t priority, MultipleWaitableEventPtr event)
{
  ScopedLock _(waitingJobsLock);
  if (waitingJobs.size() <= priority)
    waitingJobs.resize(priority + 1);
  waitingJobs[priority].push_back(new SignalingJob(job, event));
}

void ThreadPool::addJobs(const std::vector<JobPtr>& jobs, size_t priority, MultipleWaitableEventPtr event)
{
  ScopedLock _(waitingJobsLock);
  if (waitingJobs.size() <= priority)
    waitingJobs.resize(priority + 1);

  std::list<JobPtr>& waiting = waitingJobs[priority];
  for (size_t i = 0; i < jobs.size(); ++i)
    waiting.push_back(new SignalingJob(jobs[i], event));
}

JobPtr ThreadPool::popJob()
{
  ScopedLock _(waitingJobsLock);
  for (int i = waitingJobs.size() - 1; i >= 0; --i)
  {
    std::list<JobPtr>& jobs = waitingJobs[i];
    if (jobs.size())
    {
      JobPtr res = jobs.front();
      jobs.pop_front();
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

void ThreadPool::update()
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
    JobPtr job = popJob();
    if (job)
    {
      if (verbose)
        MessageCallback::info(T("Start Job: ") + job->getName());
      startThreadForJob(job);
    }
    else
      break;
  }

  if (verbose)
  {
    static int counter = 0;
    if (++counter % 1000 == 0)
    {
      MessageCallback::info(T("\n==============="));
      writeCurrentState(std::cout);
      MessageCallback::info(String::empty);
    }
  }
}

void ThreadPool::addJobAndWaitExecution(JobPtr job, size_t priority, bool callUpdateWhileWaiting)
{
  MultipleWaitableEventPtr event = new MultipleWaitableEvent();
  addJob(job, priority, event);
  if (callUpdateWhileWaiting)
    while (!event->wait(1, 1))
      update();
  else
    event->wait(1);
}

void ThreadPool::addJobsAndWaitExecution(const std::vector<JobPtr>& jobs, size_t priority, bool callUpdateWhileWaiting)
{
  Thread* currentThread = Thread::getCurrentThread();
  MultipleWaitableEventPtr event = new MultipleWaitableEvent();
  {
    ScopedLock _(threadsLock);
    addJobs(jobs, priority, event);
    if (currentThread)
      waitingThreads.insert(currentThread);
  }

  if (callUpdateWhileWaiting)
    while (!event->wait(jobs.size(), 1))
      update();
  else
    event->wait(jobs.size());
  
  {
    ScopedLock _(threadsLock);
    if (currentThread)
      waitingThreads.erase(currentThread);
    update();
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

void ThreadPool::startThreadForJob(JobPtr job)
{
  ScopedLock _(threadsLock);
  Thread* res = new RunJobThread(job);
  res->startThread();
  threads.push_back(res);
}

void ThreadPool::writeCurrentState(std::ostream& ostr)
{
  ScopedLock _1(threadsLock);
  ScopedLock _2(waitingJobsLock);

  size_t numWaitingJobs = 0;
  for (size_t i = 0; i < waitingJobs.size(); ++i)
    numWaitingJobs += waitingJobs[i].size();

  ostr << numCpus << " cpus, " << getNumWaitingThreads() << " paused threads, "
      << getNumRunningThreads() << " running threads, "
      << numWaitingJobs << " waiting jobs " << std::endl;

  for (size_t i = 0; i < threads.size(); ++i)
  {
    RunJobThread* thread = dynamic_cast<RunJobThread* >(threads[i]);
    jassert(thread);
    JobPtr job = thread->getJob();
    ostr << (isThreadWaiting(thread) ? "W" : "A") << " " << job->getName() << std::endl << "  " << job->getCurrentStatus() << std::endl << std::endl;
  }
  if (numWaitingJobs)
  {
    ostr << "- Queue - " << std::endl;
    for (int i = waitingJobs.size() - 1; i >= 0; --i)
    {
      const std::list<JobPtr>& jobs = waitingJobs[i];
      for (std::list<JobPtr>::const_iterator it = jobs.begin(); it != jobs.end(); ++it)
        ostr << "[" << i << "] " << (*it)->getName() << std::endl;
    }
  }
}
