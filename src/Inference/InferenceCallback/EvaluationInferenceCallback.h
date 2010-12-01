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
# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Inference/Inference.h>

namespace lbcpp
{

class EvaluationInferenceCallback : public ExecutionCallback
{
public:
  EvaluationInferenceCallback(const InferencePtr& inference, const EvaluatorPtr& evaluator)
    : inference(inference), evaluator(evaluator) {}
  EvaluationInferenceCallback() {}

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {
    if (inference == this->inference && output.exists() && supervision.exists())
    {
      ScopedLock _(evaluatorLock);
      evaluator->addPrediction(getContext(), output, supervision);
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
