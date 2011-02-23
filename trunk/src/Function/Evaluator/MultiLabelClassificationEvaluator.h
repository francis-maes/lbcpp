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
    : hammingLoss(new ScalarVariableMean(T("Hamming"))),
      accuracy(new ScalarVariableMean(T("Accuracy"))),
      precision(new ScalarVariableMean(T("Precision"))),
      recall(new ScalarVariableMean(T("Recall")))
  {}
  
  virtual double getScoreToMinimize() const
    {return -accuracy->getMean();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    res.push_back(std::make_pair(T("HammingLoss"), hammingLoss->getMean()));
    res.push_back(std::make_pair(T("Accuracy"), accuracy->getMean()));
    res.push_back(std::make_pair(T("Precision"), precision->getMean()));
    res.push_back(std::make_pair(T("Recall"), recall->getMean()));
  }
  
  void pushHammingLoss(double value)
    {hammingLoss->push(value);}
  
  void pushAccuracy(double value)
    {accuracy->push(value);}
  
  void pushPrecision(double value)
    {precision->push(value);}
  
  void pushRecall(double value)
    {recall->push(value);}
  
  virtual String toString() const
  {
    if (!hammingLoss->getCount())
      return String::empty;
    
    return getName() + T(" hammingLoss: ") + String(hammingLoss->getMean()) + 
    T(" acc: ") + String(accuracy->getMean()) + 
    T(" prec: ") + String(precision->getMean()) + 
    T(" rec: ") + String(recall->getMean());
  }

protected:
  friend class MultiLabelClassificationScoreObjectClass;
  
  ScalarVariableMeanPtr hammingLoss;
  ScalarVariableMeanPtr accuracy;
  ScalarVariableMeanPtr precision;
  ScalarVariableMeanPtr recall;
};

typedef ReferenceCountedObjectPtr<MultiLabelClassificationScoreObject> MultiLabelClassificationScoreObjectPtr;

class MultiLabelClassificationEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictedElementsType() const
    {return objectClass;}
  
  virtual TypePtr getRequiredSupervisionElementsType() const
    {return objectClass;}
  
protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new MultiLabelClassificationScoreObject();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct, ScoreObjectPtr& result) const
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
