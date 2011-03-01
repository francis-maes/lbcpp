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
  AddBiasBatchLearner(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore)
    : scoreToOptimize(scoreToOptimize) {}

  virtual TypePtr getRequiredFunctionType() const
    {return addBiasLearnableFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const AddBiasLearnableFunctionPtr& function = f.staticCast<AddBiasLearnableFunction>();

    ROCScoreObject roc;
    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      const ObjectPtr& example = trainingData[i];
      bool isPositive;
      if (convertSupervisionVariableToBoolean(example->getVariable(1), isPositive))
        roc.addPrediction(context, example->getVariable(0).getDouble(), isPositive);
    }
    for (size_t i = 0; i < validationData.size(); ++i)
    {
      const ObjectPtr& example = validationData[i];
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
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_ADD_BIAS_BATCH_LEARNER_H_
