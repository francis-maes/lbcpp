/*-----------------------------------------.---------------------------------.
| Filename: EvaluationInference.h          | Evaluate-Inference Inference    |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 01:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_EVALUATION_H_
# define LBCPP_INFERENCE_META_EVALUATION_H_

# include "RunOnSupervisedExamplesInference.h"
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class EvaluationInference : public RunOnSupervisedExamplesParallelInference
{
public:
  EvaluationInference(InferencePtr inference, EvaluatorPtr evaluator)
    : RunOnSupervisedExamplesParallelInference(inference), evaluator(evaluator) {}
  EvaluationInference() {}

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType;}

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
  {
    size_t n = state->getNumSubInferences();
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = state->getSubSupervision(i);
      Variable output = state->getSubOutput(i);
      if (supervision.exists() && output.exists())
        evaluator->addPrediction(context, output, supervision);
    }

    return evaluator->getDefaultScore();
  }

  virtual String getProgressionUnit() const
    {return T("Examples");}

protected:
  friend class EvaluationInferenceClass;

  EvaluatorPtr evaluator;
};


}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_EVALUATION_H_
