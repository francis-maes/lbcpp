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
  MultiThreadedInferenceContext(size_t numCpus)
    : pool(new ThreadPool(numCpus)) {}
  MultiThreadedInferenceContext() {}

  virtual InferenceStackPtr getCurrentStack() const
    {jassert(false); return InferenceStackPtr();}

  virtual Variable run(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    Variable output;
    ThreadPoolJob* job = new RunInferenceJob(refCountedPointerFromThis(this), pool, InferenceStackPtr(), inference, input, supervision, output, returnCode);
    pool->addJobAndWaitExecution(job);
    return output;
  }

protected:
  friend class MultiThreadedInferenceContextClass;

  ThreadPoolPtr pool;

  virtual Variable runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {jassert(false); return Variable();}

  virtual Variable runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {jassert(false); return Variable();}

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {jassert(false); return Variable();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_MULTI_THREADED_H_
