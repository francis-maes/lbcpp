/*-----------------------------------------.---------------------------------.
| Filename: InferenceThreadPoolJob.h       | Inference ThreadPoolJobs        |
| Author  : Francis Maes                   |                                 |
| Started : 20/09/2010 19:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_THREAD_POOL_JOB_H_
# define LBCPP_INFERENCE_CONTEXT_THREAD_POOL_JOB_H_

# include <lbcpp/Inference/InferenceContext.h>
# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp
{

using juce::ThreadPoolJob;
using juce::Thread;

extern InferenceContextPtr jobThreadInferenceContext(InferenceContextPtr parentContext, Thread* thread, ThreadPoolPtr pool, InferenceStackPtr stack);

class InferenceJob : public ThreadPoolJob
{
public:
  InferenceJob(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, const String& name)
    : ThreadPoolJob(name), parentContext(parentContext), pool(pool), stack(stack) {}

protected:
  InferenceContextPtr createContext() const
    {Thread* thread = Thread::getCurrentThread(); jassert(thread); return jobThreadInferenceContext(parentContext, thread, pool, stack);}

  InferenceContextPtr parentContext;
  ThreadPoolPtr pool;
  InferenceStackPtr stack;
};

class RunInferenceJob : public InferenceJob
{
public:
  RunInferenceJob(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, InferencePtr inference, const Variable& input, const Variable& supervision, Variable& output, Inference::ReturnCode& returnCode)
    : InferenceJob(parentContext, pool, stack, inference->toString()), inference(inference), input(input), supervision(supervision), output(output), returnCode(returnCode) {}

  virtual JobStatus runJob()
  {
    output = createContext()->run(inference, input, supervision, returnCode);
    return jobHasFinished;
  }

private:
  InferencePtr inference;
  Variable input;
  Variable supervision;
  Variable& output;
  Inference::ReturnCode& returnCode;
};

class RunParallelInferencesJob : public InferenceJob
{
public:
  RunParallelInferencesJob(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, ParallelInferencePtr inference, ParallelInferenceStatePtr state, size_t beginIndex, size_t endIndex, Thread* originatingThread)
    : InferenceJob(parentContext, pool, stack, String::empty), inference(inference), state(state), beginIndex(beginIndex), endIndex(endIndex), returnCode(Inference::finishedReturnCode), originatingThread(originatingThread)
  {
    String interval((int)beginIndex);
    if (endIndex != beginIndex + 1)
      interval += T(":") + String((int)(endIndex - 1));
    setJobName(inference->toString() + T("[") + interval + T("]"));
  }

  virtual JobStatus runJob()
  {
    InferenceContextPtr context = createContext();
    for (size_t i = beginIndex; i < endIndex; ++i)
    {
      if (shouldExit())
        return jobHasFinished;
      Variable subOutput;
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        returnCode = Inference::finishedReturnCode;
        subOutput = context->run(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
        if (returnCode == Inference::errorReturnCode)
        {
          MessageCallback::error("ParallelMultiThreadedInferenceJob::execute", "Could not finish sub inference");
          return jobHasFinished; 
        }
      }
      state->setSubOutput(i, subOutput);
    }

    if (state->haveAllOutputsBeenSet())
      originatingThread->notify();

    return jobHasFinished;
  }

  Inference::ReturnCode getReturnCode() const
    {return returnCode;}

private:
  ParallelInferencePtr inference;
  ParallelInferenceStatePtr state;
  size_t beginIndex;
  size_t endIndex;
  Inference::ReturnCode returnCode;
  Thread* originatingThread;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_THREAD_POOL_JOB_H_