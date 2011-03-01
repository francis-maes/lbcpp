/*-----------------------------------------.---------------------------------.
| Filename: MultiLabelClassificationEval..h| MultiLabel Classification       |
| Author  : Francis Maes                   |   Evaluator                     |
| Started : 22/02/2011 14:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_
# define LBCPP_FUNCTION_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_

# include <lbcpp/Function/Evaluator.h>
# include "Utilities.h"

namespace lbcpp
{

class MultiLabelClassificationScoreObject : public ScoreObject
{
public:
  MultiLabelClassificationScoreObject()
    : hammingLoss(0.0),
      accuracy(0.0),
      precision(0.0),
      recall(0.0),
      hammingLossVector(new ScalarVariableMean(T("Hamming"))),
      accuracyVector(new ScalarVariableMean(T("Accuracy"))),
      precisionVector(new ScalarVariableMean(T("Precision"))),
      recallVector(new ScalarVariableMean(T("Recall")))
  {}
  
  virtual double getScoreToMinimize() const
    {return -accuracy;}
  
  void pushHammingLoss(double value)
    {hammingLossVector->push(value);}
  
  void pushAccuracy(double value)
    {accuracyVector->push(value);}
  
  void pushPrecision(double value)
    {precisionVector->push(value);}
  
  void pushRecall(double value)
    {recallVector->push(value);}
  
  virtual String toString() const
  {
    if (!hammingLossVector->getCount())
      return String::empty;
    
    return getName() + T(" hammingLoss: ") + String(hammingLoss) + 
    T(" acc: ") + String(accuracy) + 
    T(" prec: ") + String(precision) + 
    T(" rec: ") + String(recall);
  }
  
  void finalize()
  {
    hammingLoss = hammingLossVector->getMean();
    accuracy = accuracyVector->getMean();
    precision = precisionVector->getMean();
    recall = recallVector->getMean();
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ScoreObject::clone(context, target);
    ReferenceCountedObjectPtr<MultiLabelClassificationScoreObject> res = target.staticCast<MultiLabelClassificationScoreObject>();
    if (hammingLossVector)
      res->hammingLossVector = hammingLossVector->cloneAndCast<ScalarVariableMean>(context);
    if (accuracyVector)
      res->accuracyVector = accuracyVector->cloneAndCast<ScalarVariableMean>(context);
    if (precisionVector)
      res->precisionVector = precisionVector->cloneAndCast<ScalarVariableMean>(context);
    if (recallVector)
      res->recallVector = recallVector->cloneAndCast<ScalarVariableMean>(context);
  }

protected:
  friend class MultiLabelClassificationScoreObjectClass;
  
  double hammingLoss;
  double accuracy;
  double precision;
  double recall;
  
  ScalarVariableMeanPtr hammingLossVector;
  ScalarVariableMeanPtr accuracyVector;
  ScalarVariableMeanPtr precisionVector;
  ScalarVariableMeanPtr recallVector;
};

typedef ReferenceCountedObjectPtr<MultiLabelClassificationScoreObject> MultiLabelClassificationScoreObjectPtr;

class MultiLabelClassificationEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return objectClass;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return objectClass;}
  
protected:
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context) const
    {return new MultiLabelClassificationScoreObject();}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score) const
    {score.staticCast<MultiLabelClassificationScoreObject>()->finalize();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct, const   ScoreObjectPtr& result) const
  {
    const ObjectPtr& predictedObject = predicted.getObject();
    const ObjectPtr& correctObject = correct.getObject();
    MultiLabelClassificationScoreObjectPtr score = result.staticCast<MultiLabelClassificationScoreObject>();
    jassert(predictedObject->getClass() == correctObject->getClass());

    size_t n = predictedObject->getNumVariables();
    BinaryClassificationConfusionMatrix confusionMatrix;
    for (size_t i = 0; i < n; ++i)
    {
      bool p = false, c = false;
      confusionMatrix.convertToBoolean(context, predictedObject->getVariable(i), p);
      confusionMatrix.convertToBoolean(context, correctObject->getVariable(i), c);
      confusionMatrix.addPrediction(p, c);
    }
    score->pushHammingLoss(1.0 - confusionMatrix.computeAccuracy());
    score->pushAccuracy(confusionMatrix.getTruePositives() / (double)(confusionMatrix.getSampleCount() - confusionMatrix.getTrueNegatives()));
    score->pushPrecision(confusionMatrix.computePrecision());
    score->pushRecall(confusionMatrix.computeRecall());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_
