/*-----------------------------------------.---------------------------------.
| Filename: ThreadPool.h                   | Thread Pool                     |
| Author  : Francis Maes                   |                                 |
| Started : 08/10/2010 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_THREAD_POOL_H_
# define LBCPP_EXECUTION_THREAD_POOL_H_

# include "../Data/Object.h"
# include "../Data/Cache.h"
# include "WorkUnit.h"

# include <list>

namespace lbcpp
{

class MultipleWaitableEvent;
typedef ReferenceCountedObjectPtr<MultipleWaitableEvent> MultipleWaitableEventPtr;

class ThreadPool : public Object
{
public:
  ThreadPool(size_t numCpus = 1, bool verbose = false);
  virtual ~ThreadPool();

  void update(ExecutionContext& context);

  size_t getNumCpus() const
    {return numCpus;}

  size_t getNumFreeCpus() const
    {return juce::jmax(0, (int)getNumCpus() - (int)getNumRunningThreads());}

  size_t getNumWaitingThreads() const;
  size_t getNumRunningThreads() const;
  size_t getNumThreads() const;

  void addWorkUnitAndWaitExecution(WorkUnitPtr workUnit, size_t priority = 0, bool callUpdateWhileWaiting = true);
  void addWorkUnitsAndWaitExecution(const std::vector<WorkUnitPtr>& workUnits, size_t priority, bool callUpdateWhileWaiting = true);

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

  CriticalSection waitingWorkUnitsLock;
  std::vector< std::list< WorkUnitPtr > > waitingWorkUnits;

  void addWorkUnit(WorkUnitPtr workUnit, size_t priority, MultipleWaitableEventPtr waitingEvent);
  void addWorkUnits(const std::vector<WorkUnitPtr>& workUnits, size_t priority, MultipleWaitableEventPtr waitingEvent);
  WorkUnitPtr popWorkUnit();

  void startThreadForWorkUnit(WorkUnitPtr workUnit);
  Thread* createThreadForWorkUnitIfAvailableCpu(WorkUnitPtr workUnit);
};

typedef ReferenceCountedObjectPtr<ThreadPool> ThreadPoolPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_THREAD_POOL_H_
