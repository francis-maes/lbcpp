/*-----------------------------------------.---------------------------------.
| Filename: ThreadOwnedInferenceContext.h    | Inference Context for an        |
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

namespace lbcpp
{

extern JobPtr parallelInferenceJob(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, ParallelInferencePtr inference, ParallelInferenceStatePtr state, size_t beginIndex, size_t endIndex, Thread* originatingThread);

class ThreadOwnedInferenceContext : public InferenceContext
{
public:
  ThreadOwnedInferenceContext(InferenceContextPtr parentContext, Thread* thread, ThreadPoolPtr pool, InferenceStackPtr stack)
    : parentContext(parentContext), thread(thread), pool(pool), stack(stack ? stack->cloneAndCast<InferenceStack>() : InferenceStackPtr(new InferenceStack()))
    {}
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
    parentContext->callPostInference(stack, input, supervision, output, returnCode);
    callPostInference(stack, input, supervision, output, returnCode);
    stack->pop();
  }

  size_t divceil(size_t num, size_t denom)
    {jassert(denom); return (size_t)ceil((double)num / (double)denom);}

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();

    size_t n = state->getNumSubInferences();
    size_t numFreeCpus = pool->getNumFreeCpus();
    size_t step = numFreeCpus > 1 ? divceil(n, numFreeCpus) : n;

    if (step == n)
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
      //std::cout << "PARALLEL " << inference->getDescription(input, supervision) << ": " << n << " sub inferences, " << step << " inferences per job" << std::endl;
  //    juce::DBG("Run Parallel Inference: " + inference->toString() + T(" num inferences: ") + String((int)n) + T(" step = ") + String((int)step));

      stackLock.enter();
      InferenceStackPtr stack = this->stack->cloneAndCast<InferenceStack>();
      stackLock.exit();

      for (size_t begin = 0; begin < n; )
      {
        size_t end = begin + step;
        if (end > n)
          end = n;
        JobPtr job = parallelInferenceJob(parentContext, pool, stack->cloneAndCast<InferenceStack>(), inference, state, begin, end, thread);
        pool->addJob(job, stack->getDepth());
        begin = end;
      }
      pool->waitThread(thread);
  //   juce::DBG("OK Run Parallel Inference: " + inference->toString());
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

  CriticalSection currentStateLock;
  InferencePtr currentInference;
  Variable currentInput;
  Variable currentSupervision;
};

typedef ReferenceCountedObjectPtr<ThreadOwnedInferenceContext> ThreadOwnedInferenceContextPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
