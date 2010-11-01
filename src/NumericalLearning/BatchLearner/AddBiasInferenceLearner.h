/*-----------------------------------------.---------------------------------.
| Filename: AddBiasInferenceeLearner.h     | Add Bias Inference Learner      |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_ADD_BIAS_H_
# define LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_ADD_BIAS_H_

# include "../Inference/AddBiasInference.h"
# include <lbcpp/Data/Pair.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class AddBiasInferenceLearner : public InferenceBatchLearner<Inference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return addBiasInferenceClass;}
  
  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();
    ROCAnalyse roc;
    size_t n = learnerInput->getNumTrainingExamples();
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = learnerInput->getTrainingExample(i);
      const ScalarFunctionPtr& loss = example.second.getObjectAndCast<ScalarFunction>();
      bool isPositiveExample = loss->compute(1.0) < loss->compute(-1.0);
      roc.addPrediction(example.first.getDouble(), isPositiveExample);
    }
    if (n)
    {
      double bestF1Score;
      double threshold = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1Score);
      MessageCallback::info(T("Best threshold F1: ") + String(threshold) + T(" (F1: ") + String(bestF1Score * 100.0) + T("%) - ") + String((int)roc.getSampleCount()) + T(" samples"));
      double bestMcc;
      threshold = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeMatthewsCorrelation, bestMcc);
      MessageCallback::info(T("Best threshold MCC: ") + String(threshold) + T(" (MCC: ") + String(bestMcc) + T(")"));

      learnerInput->getTargetInference().staticCast<AddBiasInference>()->setBias(-threshold);
    }
    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_ADD_BIAS_H_
