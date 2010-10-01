/*-----------------------------------------.---------------------------------.
| Filename: SingleThreadedInferenceContext.h| Multi Threaded Inference       |
| Author  : Francis Maes                   |                                 |
| Started : 20/09/2010 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_MULTI_THREADED_H_
# define LBCPP_INFERENCE_CONTEXT_MULTI_THREADED_H_

# include <lbcpp/Inference/Inference.h>
# include "InferenceThreadPoolJob.h"

namespace lbcpp
{

class MultiThreadedInferenceContext : public InferenceContext
{
public:
  MultiThreadedInferenceContext(ThreadPoolPtr pool)
    : pool(pool) {}
  MultiThreadedInferenceContext() {}

  virtual Variable run(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    Variable output;
    returnCode = Inference::finishedReturnCode;
    JobPtr job(new RunInferenceJob(refCountedPointerFromThis(this), pool, InferenceStackPtr(), inference, input, supervision, output, returnCode));
    pool->addJobAndWaitExecution(job);
    return output;
  }

protected:
  friend class MultiThreadedInferenceContextClass;

  ThreadPoolPtr pool;

  virtual void preInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
    {jassert(false);}

  virtual void postInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
    {jassert(false);}

  virtual Variable runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {jassert(false); return Variable();}

  virtual Variable runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {jassert(false); return Variable();}

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {jassert(false); return Variable();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_MULTI_THREADED_H_
