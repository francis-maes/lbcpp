/*-----------------------------------------.---------------------------------.
| Filename: SingleThreadedInferenceContext.h| Single Threaded Inference      |
| Author  : Francis Maes                   |                                 |
| Started : 20/09/2010 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_SINGLE_THREADED_H_
# define LBCPP_INFERENCE_CONTEXT_SINGLE_THREADED_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/InferenceStack.h>

namespace lbcpp
{

class SingleThreadedInferenceContext : public InferenceContext
{
public:
  SingleThreadedInferenceContext()
    : stack(new InferenceStack()) {}
  
  virtual String getName() const
    {return T("SingleThreadedInferenceContext");}

  virtual bool isMultiThread() const
    {return false;}

  // FIXME
  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual bool run(const std::vector<WorkUnitPtr>& workUnits)
  {
    for (size_t i = 0; i < workUnits.size(); ++i)
      if (!InferenceContext::run(workUnits[i]))
        return false;
    return true;
  }

  virtual void preInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    stack->push(inference);
    callPreInference(*this, stack, input, supervision, output, returnCode);
  }

  virtual void postInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    jassert(inference == stack->getCurrentInference().get());
    callPostInference(*this, stack, input, supervision, output, returnCode);
    stack->pop();
  }

private:
  friend class SingleThreadedInferenceContextClass;

  InferenceStackPtr stack;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_SINGLE_THREADED_H_

