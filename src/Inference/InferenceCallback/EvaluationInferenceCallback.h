/*-----------------------------------------.---------------------------------.
| Filename: EvaluationInferenceCallback.h  | Inference Evaluation Callback   |
| Author  : Francis Maes                   |                                 |
| Started : 02/10/2010 15:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_EVALUATION_H_
# define LBCPP_INFERENCE_CALLBACK_EVALUATION_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/InferenceStack.h>
# include <lbcpp/Inference/InferenceContext.h>

namespace lbcpp
{

class EvaluationInferenceCallback : public InferenceCallback
{
public:
  EvaluationInferenceCallback(InferencePtr inference, EvaluatorPtr evaluator)
    : inference(inference), evaluator(evaluator) {}
  EvaluationInferenceCallback() {}

  virtual void postInferenceCallback(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == inference)
    {
      if (output.exists() && supervision.exists())
      {
        ScopedLock _(evaluatorLock);
        evaluator->addPrediction(context, output, supervision);
      }
    }
  }

private:
  friend class EvaluationInferenceCallbackClass;

  InferencePtr inference;
  CriticalSection evaluatorLock;
  EvaluatorPtr evaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_EVALUATION_H_
