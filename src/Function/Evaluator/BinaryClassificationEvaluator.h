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
# include "Utilities.h"

namespace lbcpp
{

class BinaryClassificationEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return sumType(probabilityType, booleanType);}

protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new BinaryClassificationConfusionMatrix();}
  
  virtual void finalizeScoreObject(ScoreObjectPtr& score) const
    {score.staticCast<BinaryClassificationConfusionMatrix>()->finalize();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<BinaryClassificationConfusionMatrix>()->addPredictionIfExists(context, predictedObject, correctObject);}
};

class ROCAnalysisScoreObject : public ScoreObject
{
public:
  virtual double getScoreToMinimize() const
  {
    double bestF1;
    roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1);
    return -bestF1;
  }
  
  void addPrediction(ExecutionContext& context, double predicted, bool correct)
    {roc.addPrediction(context, predicted, correct);}
  
  
  
private:
  ROCAnalyse roc;
};

class ROCAnalysisEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return booleanType;}

protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new ROCAnalyse();}
  
  virtual void finalizeScoreObject(ScoreObjectPtr& score) const
  {}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<ROCAnalysisScoreObject>()->addPrediction(context, predictedObject.getDouble(), correctObject.getBoolean());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
