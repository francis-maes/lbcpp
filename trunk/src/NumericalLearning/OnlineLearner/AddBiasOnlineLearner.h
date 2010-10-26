/*-----------------------------------------.---------------------------------.
| Filename: AddBiasOnlineLearner.h         | Add Bias Online Learner         |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_ADD_BIAS_H_
# define LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_ADD_BIAS_H_

# include "../Inference/AddBiasInference.h"
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>

namespace lbcpp
{

class AddBiasOnlineLearner : public UpdatableOnlineLearner
{
public:
  AddBiasOnlineLearner(UpdateFrequency updateFrequency)
    : UpdatableOnlineLearner(updateFrequency) {}
  AddBiasOnlineLearner() {}

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    if (supervision.exists())
    {
      const ScalarFunctionPtr& loss = supervision.getObjectAndCast<ScalarFunction>();
      bool isPositiveExample = loss->compute(1.0) < loss->compute(-1.0);
      roc.addPrediction(input.getDouble(), isPositiveExample);
    }
    UpdatableOnlineLearner::stepFinishedCallback(inference, input, supervision, prediction);
  }

  virtual bool isLearningStopped() const
    {return false;}

  virtual bool wantsMoreIterations() const
    {return false;}

  virtual double getCurrentLossEstimate() const
    {return 0.0;}

  virtual void update(const InferencePtr& inf)
  {
    // retrieve examples
    const AddBiasInferencePtr& inference = inf.staticCast<AddBiasInference>();
    
    if (roc.getSampleCount())
    {
      double bestF1Score;
      double threshold = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1Score);
      MessageCallback::info(T("Best threshold F1: ") + String(threshold) + T(" (F1: ") + String(bestF1Score * 100.0) + T("%) - ") + String((int)roc.getSampleCount()) + T(" samples"));
      double bestMcc;
      threshold = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeMatthewsCorrelation, bestMcc);
      MessageCallback::info(T("Best threshold MCC: ") + String(threshold) + T(" (MCC: ") + String(bestMcc) + T(")"));

      inference->setBias(-threshold);
      roc.clear();
    }
  }

protected:
  UpdateFrequency updateFrequency;
  ScalarVariableMean bestThreshold;

  ROCAnalyse roc;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_

