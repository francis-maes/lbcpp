/*-----------------------------------------.---------------------------------.
| Filename: ThreadOwnedInferenceContext.h  | Inference Context for an        |
| Author  : Francis Maes                   |   Inference job                 |
| Started : 20/09/2010 19:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
# define LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Inference/InferenceContext.h>
# include <lbcpp/Inference/InferenceStack.h>
# include <lbcpp/Data/Cache.h>

namespace lbcpp
{

extern JobPtr parallelInferenceJob(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, ParallelInferencePtr inference, ParallelInferenceStatePtr state, size_t beginIndex, size_t endIndex, bool areSubJobsAtomic);

class ThreadOwnedInferenceContext : public InferenceContext
{
public:
  ThreadOwnedInferenceContext(InferenceContextPtr parentContext, Thread* thread, ThreadPoolPtr pool, InferenceStackPtr stack, bool isAtomicJob = false)
    : parentContext(parentContext), thread(thread), pool(pool),
      stack(stack ? stack->cloneAndCast<InferenceStack>() : InferenceStackPtr(new InferenceStack())), isAtomicJob(isAtomicJob)
  {
  }

  ThreadOwnedInferenceContext() : thread(NULL) {}

  virtual void preInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (thread->threadShouldExit())
    {
      returnCode = Inference::canceledReturnCode;
      return;
    }
    {
      ScopedLock _(currentStateLock);
      currentInput = input;
      currentSupervision = supervision;
      currentInference = inference;
    }
    {
      ScopedLock _(stackLock);
      stack->push(inference);
      parentContext->callPreInference(stack, input, supervision, output, returnCode);
      callPreInference(stack, input, supervision, output, returnCode);
    }
  }

  virtual void postInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    ScopedLock _(stackLock);
    callPostInference(stack, input, supervision, output, returnCode);
    parentContext->callPostInference(stack, input, supervision, output, returnCode);
    stack->pop();
  }

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();

    size_t n = state->getNumSubInferences();
    if (n)
    {
      size_t numCpus = pool->getNumCpus();
      
      double meanRunTime = pool->getTimingsCache()->getMeanValue(inference);
      size_t step;

      // step = num sub-inferences per sub-job

      bool areSubJobsAtomic = true;
      if (isAtomicJob)
        step = n; // atomic job: do not split
      else
      {
        // minimum 1 step per sub-jobs
        // maximum 5 * numCpus sub-jobs
        // ideally 1 s per sub-job

        step = n / numCpus;
        if (!step)
          step = 1;
        if (meanRunTime)
          step = (size_t)juce::jlimit((int)step, (int)n, (int)(1000.0 / meanRunTime));
        jassert(step > 0);
        if (n / step < numCpus)
          areSubJobsAtomic = false;
      }

      if (isAtomicJob)//step == n) // TMP!
      {
        //std::cout << "Unsplitted PARALLEL " << inference->getDescription(input, supervision) << ": " << n << " sub inferences, " << step << " inferences per job" << std::endl;
        for (size_t i = 0; i < n; ++i)
        {
          Variable subOutput;
          InferencePtr subInference = state->getSubInference(i);
          if (subInference)
          {
            returnCode = Inference::finishedReturnCode;
            subOutput = run(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
            if (returnCode == Inference::errorReturnCode)
            {
              MessageCallback::error("InferenceContext::runParallelInferences", "Could not finish sub inference");
              return Variable(); 
            }
          }
          state->setSubOutput(i, subOutput);
        }
      }
      else
      {
        MessageCallback::info(T("Parallelisation: ") + inference->getDescription(input, supervision) + T("\n\t=> ") +
          String((int)n) + T(" sub inferences, ") + String((int)step) + T(" inferences per thread"));

        stackLock.enter();
        InferenceStackPtr stack = this->stack->cloneAndCast<InferenceStack>();
        stackLock.exit();

        std::vector<JobPtr> jobs;
        jobs.reserve(1 + n / step);

        for (size_t begin = 0; begin < n; )
        {
          size_t end = begin + step;
          if (end > n)
            end = n;
          jobs.push_back(parallelInferenceJob(parentContext, pool, stack->cloneAndCast<InferenceStack>(), inference, state, begin, end, areSubJobsAtomic));
          begin = end;
        }

        pool->addJobsAndWaitExecution(jobs, stack->getDepth(), false);
      }
    }
    return inference->finalizeInference(InferenceContextPtr(this), state, returnCode);
  }

  String describeCurrentState() const
  {
    ScopedLock _(currentStateLock);
    if (currentInference)
      return currentInference->getDescription(currentInput, currentSupervision);
    else
      return T("Not started");
  }

protected:
  friend class ThreadOwnedInferenceContextClass;

  InferenceContextPtr parentContext;
  Thread* thread;
  ThreadPoolPtr pool;

  CriticalSection stackLock;
  InferenceStackPtr stack;

  bool isAtomicJob;

  CriticalSection currentStateLock;
  InferencePtr currentInference;
  Variable currentInput;
  Variable currentSupervision;
};

typedef ReferenceCountedObjectPtr<ThreadOwnedInferenceContext> ThreadOwnedInferenceContextPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
