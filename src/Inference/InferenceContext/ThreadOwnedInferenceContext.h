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

extern WorkUnitPtr parallelInferenceWorkUnit(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, ParallelInferencePtr inference, ParallelInferenceStatePtr state, size_t beginIndex, size_t endIndex, bool areSubJobsAtomic);

class ThreadOwnedInferenceContext : public InferenceContext
{
public:
  ThreadOwnedInferenceContext(Thread* thread, ThreadPoolPtr pool, InferenceStackPtr stack, bool isAtomicJob = false)
    : thread(thread), pool(pool), isAtomicJob(isAtomicJob)
  {
    this->stack = stack ? stack->cloneAndCast<InferenceStack>(*this) : InferenceStackPtr(new InferenceStack());
  }

  ThreadOwnedInferenceContext() : thread(NULL) {}
  
  // FIXME !
  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual bool run(const std::vector<WorkUnitPtr>& workUnits)
    {jassert(false); return false;}

  virtual Variable run(const InferencePtr& inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return InferenceContext::run(inference, input, supervision, returnCode);}
  // -

  virtual void preInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
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
      callPreInference(*this, stack, input, supervision, output, returnCode);
    }
  }

  virtual void postInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    ScopedLock _(stackLock);
    jassert(stack->getCurrentInference() == inference);
    callPostInference(*this, stack, input, supervision, output, returnCode);
    stack->pop();
  }

  virtual Variable runParallelInference(ParallelInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(*this, input, supervision, returnCode);
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
#ifdef LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS
      if (isAtomicJob)
#else
      if (step == n)
#endif // LBCPP_DEBUG_REFCOUNT_ATOMIC_OPERATIONS
      {
        //std::cout << "Unsplitted PARALLEL " << inference->getDescription(input, supervision) << ": " << n << " sub inferences, " << step << " inferences per job" << std::endl;
        for (size_t i = 0; i < n; ++i)
        {
          Variable subOutput;
          const InferencePtr& subInference = state->getSubInference(i);
          if (subInference)
          {
            returnCode = Inference::finishedReturnCode;
            subOutput = run(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
            if (returnCode == Inference::errorReturnCode)
            {
              errorCallback("InferenceContext::runParallelInferences", "Could not finish sub inference");
              return Variable(); 
            }
          }
          state->setSubOutput(i, subOutput);
        }
      }
      else
      {
        informationCallback(T("Parallelisation: ") + inference->getDescription(*this, input, supervision) + T("\n\t=> ") +
          String((int)n) + T(" sub inferences, ") + String((int)step) + T(" inferences per thread"));

        stackLock.enter();
        InferenceStackPtr stack = this->stack->cloneAndCast<InferenceStack>(*this);
        stackLock.exit();

        std::vector<WorkUnitPtr> workUnits;
        workUnits.reserve(1 + n / step);

        for (size_t begin = 0; begin < n; )
        {
          size_t end = begin + step;
          if (end > n)
            end = n;
          workUnits.push_back(parallelInferenceWorkUnit(refCountedPointerFromThis(this), pool,
            stack->cloneAndCast<InferenceStack>(*this), inference, state, begin, end, areSubJobsAtomic));
          begin = end;
        }

        pool->addWorkUnitsAndWaitExecution(workUnits, stack->getDepth(), false);
      }
    }
    return inference->finalizeInference(*this, state, returnCode);
  }

  String describeCurrentState()
  {
    ScopedLock _(currentStateLock);
    if (currentInference)
      return currentInference->getDescription(*this, currentInput, currentSupervision);
    else
      return T("Not started");
  }

protected:
  friend class ThreadOwnedInferenceContextClass;

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
