/*-----------------------------------------.---------------------------------.
| Filename: JobThreadInferenceContext.h    | Inference Context for an        |
| Author  : Francis Maes                   |   Inference job                 |
| Started : 20/09/2010 19:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
# define LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Inference/InferenceStack.h>

namespace lbcpp
{

class JobThreadInferenceContext : public InferenceContext
{
public:
  JobThreadInferenceContext(Thread* thread, ThreadPoolPtr pool, InferenceStackPtr stack)
    : thread(thread), pool(pool), stack(stack ? stack->cloneAndCast<InferenceStack>() : InferenceStackPtr(new InferenceStack()))
    {}
  JobThreadInferenceContext() : thread(NULL) {}

  virtual InferenceStackPtr getCurrentStack() const
    {return stack;}

  virtual Variable run(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (thread->threadShouldExit())
    {
      returnCode = Inference::canceledReturnCode;
      return Variable();
    }
    return InferenceContext::run(inference, input, supervision, returnCode);
  }

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();
    
    size_t n = state->getNumSubInferences();
    size_t step = n / pool->getNumCpus();
    if (!step)
      step = 1;

    for (size_t begin = 0; begin < n; )
    {
      size_t end = begin + step;
      if (end > n)
        end = n;
      ThreadPoolJob* job = new RunParallelInferencesJob(pool, stack, inference, state, begin, end, thread);
      pool->addJob(job, stack->getDepth());
      begin = end;
    }
    pool->waitThread(thread);
    return inference->finalizeInference(InferenceContextPtr(this), state, returnCode);
  }

protected:
  friend class JobThreadInferenceContextClass;

  Thread* thread;
  ThreadPoolPtr pool;
  InferenceStackPtr stack;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
