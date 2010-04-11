/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.h            | Inference Callback base class   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_H_

# include "../InferenceStep/InferenceStep.h"
# include "../InferenceCallback/InferenceContext.h"

namespace lbcpp
{

class InferenceStack;
typedef ReferenceCountedObjectPtr<InferenceStack> InferenceStackPtr;

class InferenceCallback : public Object
{
public:
  typedef InferenceStep::ReturnCode ReturnCode;

  virtual void startInferencesCallback(size_t count)
    {}

  virtual void finishInferencesCallback()
    {}

  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
    {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}

  virtual void classificationCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
    {}
};

typedef ReferenceCountedObjectPtr<InferenceCallback> InferenceCallbackPtr;

class CancelAfterStepCallback : public InferenceCallback
{
public:
  CancelAfterStepCallback(InferenceStepPtr step)
    : step(step) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == step)
      returnCode = InferenceStep::canceledReturnCode;
  }

private:
  InferenceStepPtr step;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CALLBACK_H_
