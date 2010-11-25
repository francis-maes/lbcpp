/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.h            | Inference Callback base class   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_H_

# include "Inference.h"
# include "../Execution/ExecutionCallback.h"

namespace lbcpp
{

class InferenceCallback : public ExecutionCallback
{
public:
  typedef Inference::ReturnCode ReturnCode;

  // this function may modify the input or the supervision
  // it may also set an output, which causes the current inference step to be skipped
  // the function may also set a returnCode != Inference::finishedReturnCode to skip the inference step
  virtual void preInferenceCallback(ExecutionContext& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
    {}

  virtual void postInferenceCallback(ExecutionContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
    {}
};

extern InferenceCallbackPtr cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep);
extern InferenceCallbackPtr cancelAfterStepCallback(InferencePtr lastStepBeforeBreak);
extern InferenceCallbackPtr evaluationInferenceCallback(InferencePtr inference, EvaluatorPtr evaluator);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CALLBACK_H_
