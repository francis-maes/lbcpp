/*-----------------------------------------.---------------------------------.
| Filename: CancelAfterStepCallback.h      | A callback that breaks inference|
| Author  : Francis Maes                   |  after a given step.            |
| Started : 09/04/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_CANCEL_AFTER_STEP_CALLBACK_H_
# define LBCPP_INFERENCE_CONTEXT_CANCEL_AFTER_STEP_CALLBACK_H_

# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/Inference/InferenceStack.h>

namespace lbcpp
{

class CancelAfterStepCallback : public InferenceCallback
{
public:
  CancelAfterStepCallback(InferencePtr step)
    : stepName(step->getName()) {}
  CancelAfterStepCallback() {}

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference()->getName() == stepName)
      returnCode = Inference::canceledReturnCode;
  }

protected:
  String stepName;

  virtual bool load(InputStream& istr)
    {return InferenceCallback::load(istr) && lbcpp::read(istr, stepName);}

  virtual void save(OutputStream& ostr) const
    {InferenceCallback::save(ostr); lbcpp::write(ostr, stepName);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_CANCEL_AFTER_STEP_CALLBACK_H_
