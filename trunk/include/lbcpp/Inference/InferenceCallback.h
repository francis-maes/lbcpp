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
# include "../ObjectPredeclarations.h"

namespace lbcpp
{

class InferenceCallback : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual void startInferencesCallback(size_t count)
    {}

  virtual void finishInferencesCallback()
    {}

  // this function may modify the input or the supervision
  // it may also set an output, which causes the current inference step to be skipped
  // the function may also set a returnCode != Inference::finishedReturnCode to skip the inference step
  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}
};

extern InferenceCallbackPtr cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep);
extern InferenceCallbackPtr cancelAfterStepCallback(InferencePtr lastStepBeforeBreak);

class InferenceOnlineLearnerCallback : public InferenceCallback
{
public:
  InferenceOnlineLearnerCallback(InferencePtr inference, InferenceOnlineLearnerPtr learner)
    : inference(inference), learner(learner) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);
  virtual void finishInferencesCallback();

private:
  InferencePtr inference;
  InferenceOnlineLearnerPtr learner;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CALLBACK_H_
