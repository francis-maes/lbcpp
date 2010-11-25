/*-----------------------------------------.---------------------------------.
| Filename: InferenceRelatedJobs.h         | Inference Related Jobs          |
| Author  : Francis Maes                   |                                 |
| Started : 20/09/2010 19:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_RELATED_JOBS_H_
# define LBCPP_INFERENCE_CONTEXT_RELATED_JOBS_H_

# include <lbcpp/Data/Cache.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Execution/ExecutionContext.h>
# include <lbcpp/Inference/InferenceContext.h>
# include <lbcpp/Inference/ParallelInference.h>
# include "ThreadOwnedInferenceContext.h"

namespace lbcpp
{

class InferenceRelatedWorkUnit : public WorkUnit
{
public:
  InferenceRelatedWorkUnit(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, const String& name = T("Unnamed"), bool areSubJobsAtomic = false)
    : WorkUnit(name), parentContext(parentContext), pool(pool), stack(stack), areSubJobsAtomic(areSubJobsAtomic) {}

  /*virtual String getCurrentStatus() const
  {
    ScopedLock _(contextLock);
    return context ? context->describeCurrentState() : T("Not started yet");
  }*/

  virtual bool run(ExecutionContext& executionContext)
  {
    ScopedLock _(contextLock);
    Thread* thread = Thread::getCurrentThread();
    jassert(thread);
    context = new ThreadOwnedInferenceContext(thread, pool, stack, areSubJobsAtomic);
    context->setCallbacks(parentContext->getCallbacks());
    jassert(context);
    return true;
  }

protected:
  InferenceContextPtr parentContext;
  ThreadPoolPtr pool;
  InferenceStackPtr stack;
  bool areSubJobsAtomic;

  CriticalSection contextLock;
  ThreadOwnedInferenceContextPtr context;
};

class RunInferenceWorkUnit : public InferenceRelatedWorkUnit
{
public:
  RunInferenceWorkUnit(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, InferencePtr inference, const Variable& input, const Variable& supervision, Variable& output, Inference::ReturnCode& returnCode)
    : InferenceRelatedWorkUnit(parentContext, pool, stack), inference(inference), input(input), supervision(supervision), output(output), returnCode(returnCode)
  {
    setName(inference->getDescription(*parentContext, input, supervision));
  }

  virtual bool run(ExecutionContext& executionContext)
  {
    if (!InferenceRelatedWorkUnit::run(executionContext))
      return false;
    output = context->runInference(inference, input, supervision, returnCode);
    return true;
  }

private:
  InferencePtr inference;
  Variable input;
  Variable supervision;
  Variable& output;
  Inference::ReturnCode& returnCode;
};

class RunParallelInferencesWorkUnit : public InferenceRelatedWorkUnit
{
public:
  RunParallelInferencesWorkUnit(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, ParallelInferencePtr inference, ParallelInferenceStatePtr state, size_t beginIndex, size_t endIndex, bool areSubJobsAtomic)
    : InferenceRelatedWorkUnit(parentContext, pool, stack, String::empty, areSubJobsAtomic), inference(inference), state(state), beginIndex(beginIndex), endIndex(endIndex), returnCode(Inference::finishedReturnCode), areSubJobsAtomic(areSubJobsAtomic)
  {
    if (endIndex == beginIndex + 1)
    {
      const InferencePtr& subInference = state->getSubInference(beginIndex);
      stack->push(subInference);
      setName(subInference->getDescription(*parentContext, state->getSubInput(beginIndex), state->getSubSupervision(beginIndex)));
      stack->pop();
    }
    else
    {
      setName(inference->getDescription(*parentContext, state->getInput(), state->getSupervision()) +
        T("[") + String((int)beginIndex) + T(":") + String((int)(endIndex - 1)) + T("]"));
    }
  }

  virtual bool run(ExecutionContext& executionContext)
  {
    if (!InferenceRelatedWorkUnit::run(executionContext))
      return false;

    for (size_t i = beginIndex; i < endIndex; ++i)
    {
      if (executionContext.isCanceled())
        return true;

      Variable subOutput;
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        double startingTime = Time::getMillisecondCounterHiRes();
        //juce::uint32 checkDebug = Time::getMillisecondCounter();
        
        returnCode = Inference::finishedReturnCode;
        subOutput = context->runInference(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
        if (returnCode == Inference::errorReturnCode)
        {
          executionContext.errorCallback(T("RunParallelInferencesWorkUnit"), T("Could not finish sub inference"));
          return false; 
        }

        double deltaTime = Time::getMillisecondCounterHiRes() - startingTime;
        //std::cout << "Delta Time: " << deltaTime << " check: " << Time::getMillisecondCounter() - checkDebug << std::endl;
        pool->getTimingsCache()->addValue(inference, deltaTime);
      }
      state->setSubOutput(i, subOutput);
    }
//    executionContext.informationCallback(T("[") + String((int)beginIndex) + T(", ") + String((int)endIndex - 1) + 
//      T("] Mean Execution Time: ") + inference->getName() + " ==> " + String(pool->getTimingsCache()->getMeanValue(inference)));
    return true;
  }

  Inference::ReturnCode getReturnCode() const
    {return returnCode;}

private:
  ParallelInferencePtr inference;
  ParallelInferenceStatePtr state;
  size_t beginIndex;
  size_t endIndex;
  Inference::ReturnCode returnCode;
  bool areSubJobsAtomic;
};

WorkUnitPtr parallelInferenceWorkUnit(InferenceContextPtr parentContext, ThreadPoolPtr pool, InferenceStackPtr stack, ParallelInferencePtr inference, ParallelInferenceStatePtr state, size_t beginIndex, size_t endIndex, bool areSubJobsAtomic)
  {return WorkUnitPtr(new RunParallelInferencesWorkUnit(parentContext, pool, stack, inference, state, beginIndex, endIndex, areSubJobsAtomic));}

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_RELATED_JOBS_H_
