/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationConfusio..h| Binary Classification           |
| Author  : Francis Maes                   |   Confusion Matrix Evaluator    |
| Started : 22/02/2011 14:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
# define LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/Numerical.h> // for convertSupervisionVariableToBoolean
# include "Utilities.h"

namespace lbcpp
{

class BinaryClassificationEvaluator : public SupervisedEvaluator
{
public:
  BinaryClassificationEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore,
                                double threshold = 0.5f)
    : scoreToOptimize(scoreToOptimize), threshold(threshold) {}
  
  virtual TypePtr getRequiredPredictionType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return sumType(probabilityType, booleanType);}

protected:
  friend class BinaryClassificationEvaluatorClass;
  
  BinaryClassificationScore scoreToOptimize;
  double threshold;
  
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new BinaryClassificationConfusionMatrix(scoreToOptimize, threshold);}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, const ScoreObjectPtr& result) const
    {result.staticCast<BinaryClassificationConfusionMatrix>()->addPredictionIfExists(predictedObject, correctObject);}
};

class BinaryClassificationCurveEvaluator : public SupervisedEvaluator
{
public:
  BinaryClassificationCurveEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore, bool saveConfusionMatrices = false)
    : scoreToOptimize(scoreToOptimize), saveConfusionMatrices(saveConfusionMatrices) {}

  virtual TypePtr getRequiredPredictionType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return booleanType;}

protected:
  friend class BinaryClassificationCurveEvaluatorClass;

  BinaryClassificationScore scoreToOptimize;
  bool saveConfusionMatrices;

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new BinaryClassificationCurveScoreObject(scoreToOptimize);}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct, const ScoreObjectPtr& scores) const
    {scores.staticCast<BinaryClassificationCurveScoreObject>()->addPrediction(predicted, correct);}

  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
    {scores.staticCast<BinaryClassificationCurveScoreObject>()->finalize(saveConfusionMatrices);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
