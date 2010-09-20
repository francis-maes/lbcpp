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
using namespace lbcpp;

/*
** InferenceContext
*/
Variable InferenceContext::run(InferencePtr inference, const Variable& in, const Variable& sup, ReturnCode& returnCode)
{
  InferenceStackPtr stack = getCurrentStack();
  jassert(stack);
  stack->push(inference);

  Variable input(in);
  Variable supervision(sup);
  Variable output;
  returnCode = Inference::finishedReturnCode;
  callPreInference(stack, input, supervision, output, returnCode);
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

  callPostInference(stack, input, supervision, output, returnCode);

  stack->pop();
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
    return ObjectPtr();
  while (true)
  {
    Variable subOutput = run(state->getSubInference(), state->getSubInput(), state->getSubSupervision(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getInput();

    state->setSubOutput(subOutput);
    bool res = inference->updateInference(pthis, state, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getInput();

    if (!res)
    {
      state->setFinalState();
      break;
    }
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

#include <lbcpp/Function/Evaluator.h>

class EvaluationInferenceCallback : public InferenceCallback
{
public:
  EvaluationInferenceCallback(EvaluatorPtr evaluator)
    : evaluator(evaluator) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 2 && output && supervision)
      evaluator->addPrediction(output, supervision);
  }

private:
  EvaluatorPtr evaluator;
};

Inference::ReturnCode InferenceContext::evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferenceCallbackPtr evaluationCallback = new EvaluationInferenceCallback(evaluator);
  appendCallback(evaluationCallback);
  run(runOnSupervisedExamplesInference(inference), examples, Variable(), res);
  removeCallback(evaluationCallback);
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

/*
** RunJobThread
*/
using juce::Thread;
using juce::ThreadPoolJob;
 
class RunJobThread : public juce::Thread
{
public:
  RunJobThread(ThreadPoolJob* job)
    : Thread(job->getJobName()), job(job) {}

  virtual ~RunJobThread()
    {delete job;}

  void signalShouldExit()
  {
    job->signalJobShouldExit();
    signalThreadShouldExit();
  }

  virtual void run()
    {job->runJob();}

  juce_UseDebuggingNewOperator

private:
  ThreadPoolJob* job;
};

/*
** ThreadPool
*/
ThreadPool::ThreadPool(size_t numCpus)
  : numCpus(numCpus), numWaitingThreads(0) {}

ThreadPool::~ThreadPool()
{
  for (size_t i = 0; i < threads.size(); ++i)
    ((RunJobThread* )threads[i])->signalShouldExit();
  for (size_t i = 0; i < threads.size(); ++i)
    delete threads[i];
}

void ThreadPool::addJob(ThreadPoolJob* job, size_t priority)
{
  juce::DBG("Add Job: " + job->getJobName() + T(" priority: ") + String((int)priority));
  {
    ScopedLock _(waitingJobsLock); 
    if (waitingJobs.size() <= priority)
      waitingJobs.resize(priority + 1);
    waitingJobs[priority].push_back(job);
  }
}

ThreadPoolJob* ThreadPool::popJob()
{
  ScopedLock _(waitingJobsLock);
  for (int i = waitingJobs.size() - 1; i >= 0; --i)
  {
    std::list<ThreadPoolJob* >& jobs = waitingJobs[i];
    if (jobs.size())
    {
      ThreadPoolJob* res = jobs.front();
      jobs.pop_front();
      return res;
    }
  }
  return NULL;
}


size_t ThreadPool::getNumWaitingThreads() const
  {return numWaitingThreads;}

size_t ThreadPool::getNumRunningThreads() const
  {return getNumThreads() - numWaitingThreads;}

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
    ThreadPoolJob* job = popJob();
    if (job)
    {
      juce::DBG("Start Job: " + job->getJobName());
      startThreadForJob(job);
    }
    else
      break;
  }
}

void ThreadPool::waitThread(juce::Thread* thread)
{
  ++numWaitingThreads;
  thread->wait(-1);
  --numWaitingThreads;
}

class SignalThreadPoolJob : public ThreadPoolJob
{
public:
  SignalThreadPoolJob(ThreadPoolJob* job, juce::WaitableEvent& event)
    : ThreadPoolJob(job->getJobName()), job(job), event(event) {}
  virtual ~SignalThreadPoolJob()
    {delete job;}

  virtual JobStatus runJob()
  {
    JobStatus res = job->runJob();
    event.signal();
    return res;
  }

protected:
  ThreadPoolJob* job;
  juce::WaitableEvent& event;
};

void ThreadPool::addJobAndWaitExecution(juce::ThreadPoolJob* job, size_t priority)
{
  ScopedLock _(threadsLock); 
  juce::WaitableEvent event;
  ThreadPoolJob* signalingJob = new SignalThreadPoolJob(job, event);
  addJob(signalingJob, priority);
  while (!event.wait(5))
    update();
}

void ThreadPool::startThreadForJob(juce::ThreadPoolJob* job)
{
  ScopedLock _(threadsLock); 
  Thread* res = new RunJobThread(job);
  res->startThread();
  threads.push_back(res);
}
