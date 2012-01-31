/*-----------------------------------------.---------------------------------.
| Filename: AddBiasBatchLearner.h          | AddBias Batch Learner           |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2011 20:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_ADD_BIAS_BATCH_LEARNER_H_
# define LBCPP_LEARNING_NUMERICAL_ADD_BIAS_BATCH_LEARNER_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>
# include "AddBiasLearnableFunction.h"
# include "../../Function/Evaluator/Utilities.h"

namespace lbcpp
{

class AddBiasBatchLearner : public BatchLearner
{
public:
  AddBiasBatchLearner(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore, bool learnBiasOnValidationData = false)
    : scoreToOptimize(scoreToOptimize), learnBiasOnValidationData(learnBiasOnValidationData) {}

  virtual TypePtr getRequiredFunctionType() const
    {return addBiasLearnableFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const AddBiasLearnableFunctionPtr& function = f.staticCast<AddBiasLearnableFunction>();

    if (learnBiasOnValidationData)
      context.warningCallback(T("AddBiasBatchLearner::train"), T("Learning bias using validation data only"));
    const std::vector<ObjectPtr>& data = learnBiasOnValidationData ? validationData : trainingData;

    ROCScoreObject roc;
    for (size_t i = 0; i < data.size(); ++i)
    {
      const ObjectPtr& example = data[i];
      bool isPositive;
      if (convertSupervisionVariableToBoolean(example->getVariable(1), isPositive))
        roc.addPrediction(context, example->getVariable(0).getDouble(), isPositive);
    }

    if (!roc.getSampleCount())
    {
      context.errorCallback(T("No training examples"));
      return false;
    }

    double bestScore;
    double bestThreshold = roc.findBestThreshold(scoreToOptimize, bestScore);
    context.resultCallback(T("Best threshold"), bestThreshold);
    context.resultCallback(T("Best threshold score"), bestScore);
    function->setBias(-bestThreshold);
    return true;
  }

protected:
  friend class AddBiasBatchLearnerClass;

  BinaryClassificationScore scoreToOptimize;
  bool learnBiasOnValidationData;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_ADD_BIAS_BATCH_LEARNER_H_
