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

  virtual Variable runParallelInference(ParallelInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(*this, input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();
    
    size_t n = state->getNumSubInferences();
    for (size_t i = 0; i < n; ++i)
    {
      Variable subOutput;
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        returnCode = Inference::finishedReturnCode;
        subOutput = runInference(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
        if (returnCode == Inference::errorReturnCode)
        {
          errorCallback("InferenceContext::runParallelInferences", "Could not finish sub inference");
          return Variable(); 
        }
      }
      state->setSubOutput(i, subOutput);
    }
    return inference->finalizeInference(*this, state, returnCode);
  }

private:
  friend class SingleThreadedInferenceContextClass;

  InferenceStackPtr stack;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_SINGLE_THREADED_H_

