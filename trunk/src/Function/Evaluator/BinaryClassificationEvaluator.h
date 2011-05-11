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
  BinaryClassificationEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore)
    : scoreToOptimize(scoreToOptimize) {}
  
  virtual TypePtr getRequiredPredictionType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return sumType(probabilityType, booleanType);}

protected:
  friend class BinaryClassificationEvaluatorClass;
  
  BinaryClassificationScore scoreToOptimize;
  
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new BinaryClassificationConfusionMatrix(scoreToOptimize);}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, const ScoreObjectPtr& result) const
    {result.staticCast<BinaryClassificationConfusionMatrix>()->addPredictionIfExists(context, predictedObject, correctObject);}
};

class ROCAnalysisEvaluator : public SupervisedEvaluator
{
public:
  ROCAnalysisEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore, bool saveConfusionMatrices = false)
    : scoreToOptimize(scoreToOptimize), saveConfusionMatrices(saveConfusionMatrices) {}

  virtual TypePtr getRequiredPredictionType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return booleanType;}

protected:
  friend class ROCAnalysisEvaluatorClass;

  BinaryClassificationScore scoreToOptimize;
  bool saveConfusionMatrices;

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new ROCScoreObject(scoreToOptimize);}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct, const ScoreObjectPtr& scores) const
  {
    bool isPositive;
    if (convertSupervisionVariableToBoolean(correct, isPositive))
      scores.staticCast<ROCScoreObject>()->addPrediction(context, predicted.getDouble(), isPositive);
    else
      jassert(false);
  }

  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
    {scores.staticCast<ROCScoreObject>()->finalize(saveConfusionMatrices);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
