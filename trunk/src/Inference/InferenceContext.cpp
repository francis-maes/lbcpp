/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.cpp           | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/InferenceStack.h>
#include <lbcpp/Data/Container.h>
#include <lbcpp/Data/Cache.h>
using namespace lbcpp;

/*
** InferenceContext
*/
Variable InferenceContext::run(InferencePtr inference, const Variable& in, const Variable& sup, ReturnCode& returnCode)
{
  jassert(inference);
  Variable input(in);
  Variable supervision(sup);
  Variable output;
  returnCode = Inference::finishedReturnCode;
  preInference(inference, input, supervision, output, returnCode);
  if (returnCode == Inference::errorReturnCode)
  {
    MessageCallback::warning(T("InferenceContext::run"), T("pre-inference failed"));
    jassert(false);
    return Variable();
  }

  if (returnCode == Inference::canceledReturnCode)
    {jassert(output);}
  else if (!output)
    output = callRunInference(inference, input, supervision, returnCode);

  postInference(inference, input, supervision, output, returnCode);

  return output;
}

Variable InferenceContext::callRunInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {return inference->run(InferenceContextPtr(this), input, supervision, returnCode);}

Variable InferenceContext::runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  InferenceContextPtr pthis(this);

  DecoratorInferenceStatePtr state = inference->prepareInference(pthis, input, supervision, returnCode);
  jassert(state);
  if (returnCode != Inference::finishedReturnCode)
    return Variable();

  InferencePtr subInference = state->getSubInference();
  if (subInference)
  {
    Variable subOutput = run(subInference, state->getSubInput(), state->getSubSupervision(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();
    state->setSubOutput(subOutput);
  }

  return inference->finalizeInference(pthis, state, returnCode);
}

Variable InferenceContext::runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  InferenceContextPtr pthis(this);

  SequentialInferenceStatePtr state = inference->prepareInference(pthis, input, supervision, returnCode);
  if (!state)
    return Variable();
  while (!state->isFinal())
  {
    Variable subOutput = run(state->getSubInference(), state->getSubInput(), state->getSubSupervision(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getInput();

    state->setSubOutput(subOutput);
    bool res = inference->updateInference(pthis, state, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getInput();
    if (!res)
      state->setFinalState();
  }
  return inference->finalizeInference(pthis, state, returnCode);
}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, ContainerPtr examples)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferencePtr learner = inference->getBatchLearner();
  jassert(learner);
  if (!learner)
    return Inference::errorReturnCode;
  run(learner, Variable::pair(inference, examples), Variable(), res);
  return res;
}

Inference::ReturnCode InferenceContext::evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferenceCallbackPtr evaluationCallback = evaluationInferenceCallback(evaluator);
  appendCallback(evaluationCallback);
  run(runOnSupervisedExamplesInference(inference, true), examples, Variable(), res);
  removeCallback(evaluationCallback);
  return res;
}

Inference::ReturnCode InferenceContext::crossValidate(InferencePtr inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferencePtr cvInference(crossValidationInference(String((int)numFolds) + T("-CV"), evaluator, inferenceModel, numFolds));
  run(cvInference, examples, Variable(), res);
  return res;
}

void InferenceContext::callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->postInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::appendCallback(InferenceCallbackPtr callback)
  {jassert(callback); callbacks.push_back(callback);}

void InferenceContext::removeCallback(InferenceCallbackPtr callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      break;
    }
}

void InferenceContext::clearCallbacks()
  {callbacks.clear();}

/////////////////////////////////////////////////////////////
///////////////// Thread Pool ///////////////////////////////
/////////////////////////////////////////////////////////////

namespace lbcpp
{

/*
** MultipleWaitableEvent
*/
class MultipleWaitableEvent
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
  SignalingJob(JobPtr job, MultipleWaitableEvent& event)
    : Job(job->getName()), job(job), event(event) {}

  virtual String getCurrentStatus() const
    {return job->getCurrentStatus();}

  virtual bool runJob(String& failureReason)
  {
    bool res = job->runJob(failureReason);
    event.signal();
    return res;
  }

protected:
  JobPtr job;
  MultipleWaitableEvent& event;
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

void ThreadPool::addJob(JobPtr job, size_t priority, MultipleWaitableEvent& event)
{
  ScopedLock _(waitingJobsLock);
  if (waitingJobs.size() <= priority)
    waitingJobs.resize(priority + 1);
  waitingJobs[priority].push_back(new SignalingJob(job, event));
}

void ThreadPool::addJobs(const std::vector<JobPtr>& jobs, size_t priority, MultipleWaitableEvent& event)
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
  {ScopedLock _(threadsLock); return threads.size() - waitingThreads.size();}

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
        std::cout << "Start Job: " << job->getName() << std::endl;
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
      std::cout << std::endl << "===============" << std::endl;
      writeCurrentState(std::cout);
      std::cout << std::endl;
    }
  }
}

void ThreadPool::addJobAndWaitExecution(JobPtr job, size_t priority)
{   
  MultipleWaitableEvent event;
  addJob(job, priority, event);
  while (!event.wait(1, 1))
  {
    ScopedLock _(threadsLock);
    update();
    if (getNumRunningThreads() == 0 && getNumWaitingThreads() > 1)
    {
      std::cerr << std::endl << "Fatal Error: Not any running thread, Probable Dead Lock!!!" << std::endl;
      writeCurrentState(std::cerr);
      exit(1);
    }
  }
}

void ThreadPool::addJobsAndWaitExecution(const std::vector<JobPtr>& jobs, size_t priority)
{
  Thread* currentThread = Thread::getCurrentThread();
  MultipleWaitableEvent event;
  {
    ScopedLock _(threadsLock);
    addJobs(jobs, priority, event);
    waitingThreads.insert(currentThread);
  }
  event.wait(jobs.size());
  {
    ScopedLock _(threadsLock);
    waitingThreads.erase(currentThread);
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
