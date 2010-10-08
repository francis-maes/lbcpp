/*-----------------------------------------.---------------------------------.
| Filename: ThreadPool.h                   | Thread Pool                     |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_THREAD_POOL_H_
# define LBCPP_FUNCTION_THREAD_POOL_H_

# include "../Data/Object.h"
# include <list>

namespace lbcpp
{

class Job : public NameableObject
{
public:
  Job(const String& name)
    : NameableObject(name), jobShouldExit(false) {}
  Job() : jobShouldExit(false) {}

  virtual String getCurrentStatus() const = 0;

  virtual bool runJob(String& failureReason) = 0;

  bool shouldExit() const
    {return jobShouldExit;}

  void signalJobShouldExit()
    {jobShouldExit = true;}

private:
  friend class JobClass;

  bool volatile jobShouldExit;
};

typedef ReferenceCountedObjectPtr<Job> JobPtr;

class MultipleWaitableEvent;
typedef ReferenceCountedObjectPtr<MultipleWaitableEvent> MultipleWaitableEventPtr;

class ThreadPool : public Object
{
public:
  ThreadPool(size_t numCpus = 1, bool verbose = false);
  virtual ~ThreadPool();

  void update();

  size_t getNumCpus() const
    {return numCpus;}

  size_t getNumFreeCpus() const
    {return juce::jmax(0, (int)getNumCpus() - (int)getNumRunningThreads());}

  size_t getNumWaitingThreads() const;
  size_t getNumRunningThreads() const;
  size_t getNumThreads() const;

  void addJobAndWaitExecution(JobPtr job, size_t priority = 0, bool callUpdateWhileWaiting = true);
  void addJobsAndWaitExecution(const std::vector<JobPtr>& jobs, size_t priority, bool callUpdateWhileWaiting = true);

  bool isThreadWaiting(Thread* thread) const;

  void writeCurrentState(std::ostream& ostr);

  AverageValuesCachePtr getTimingsCache() const
    {return timingsCache;}

private:
  friend class ThreadPoolClass;

  size_t numCpus;
  bool verbose;
  AverageValuesCachePtr timingsCache;

  CriticalSection threadsLock;
  std::vector<juce::Thread* > threads;
  std::set<juce::Thread* > waitingThreads;

  CriticalSection waitingJobsLock;
  std::vector< std::list< JobPtr > > waitingJobs;

  void addJob(JobPtr job, size_t priority, MultipleWaitableEventPtr waitingEvent);
  void addJobs(const std::vector<JobPtr>& jobs, size_t priority, MultipleWaitableEventPtr waitingEvent);
  JobPtr popJob();

  void startThreadForJob(JobPtr job);
  Thread* createThreadForJobIfAvailableCpu(JobPtr job);
};

typedef ReferenceCountedObjectPtr<ThreadPool> ThreadPoolPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_FUNCTION_THREAD_POOL_H_
