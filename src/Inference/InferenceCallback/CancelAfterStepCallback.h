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
# include <lbcpp/Execution/ExecutionStack.h>

namespace lbcpp
{

class CancelAfterStepCallback : public ExecutionCallback
{
public:
  CancelAfterStepCallback(InferencePtr step)
    : stepName(step->getName()) {}
  CancelAfterStepCallback() {}

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const FunctionPtr& function, const Variable& input, const Variable& output)
  {
    jassert(false); // not implemented anymore
    //if (stack->getCurrentInference()->getName() == stepName)
    //  returnCode = Inference::canceledReturnCode;
  }

protected:
  String stepName;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_CANCEL_AFTER_STEP_CALLBACK_H_
