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

namespace lbcpp
{

class BinaryClassificationConfusionScoreObject : public ScoreObject
{
public:
  virtual double getScoreToMinimize() const
  {
    double precision, recall, f1score;
    confusionMatrix.computePrecisionRecallAndF1(precision, recall, f1score);
    return -f1score;
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    double precision = 0.0, recall = 0.0, f1 = 0.0, mcc = 0.0, accuracy = 0.0;
    
    if (confusionMatrix.getSampleCount())
    {
      confusionMatrix.computePrecisionRecallAndF1(precision, recall, f1);
      mcc = confusionMatrix.computeMatthewsCorrelation();
      accuracy = confusionMatrix.computeAccuracy();
    }
    res.push_back(std::make_pair(T("Precision"), precision));
    res.push_back(std::make_pair(T("Recall"), recall));
    res.push_back(std::make_pair(T("F1"), f1));
    res.push_back(std::make_pair(T("MCC"), mcc));
    res.push_back(std::make_pair(T("Accuracy"), accuracy));
  }
  
  void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct)
    {confusionMatrix.addPredictionIfExists(context, predicted, correct);}
  
  virtual String toString() const
  {
    if (!confusionMatrix.getSampleCount())
      return String::empty;
    
    double precision, recall, f1score;
    confusionMatrix.computePrecisionRecallAndF1(precision, recall, f1score);
    
    double accuracy = confusionMatrix.computeAccuracy();
    
    return getName() + T(" (") + String((int)confusionMatrix.getSampleCount()) + T(" examples)\n") + confusionMatrix.toString()
    + T("ACC = ") + String(accuracy * 100.0, 2)
    + T("% P = ") + String(precision * 100.0, 2)
    + T("% R = ") + String(recall * 100.0, 2)
    + T("% F1 = ") + String(f1score * 100.0, 2)
    + T("% MCC = ") + String(confusionMatrix.computeMatthewsCorrelation(), 4);
  }

protected:
  BinaryClassificationConfusionMatrix confusionMatrix;
};

class BinaryClassificationConfusionEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictedElementsType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionElementsType() const
    {return sumType(probabilityType, booleanType);}

protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new BinaryClassificationConfusionScoreObject();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<BinaryClassificationConfusionScoreObject>()->addPrediction(context, predictedObject, correctObject);}
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

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {roc.getScores(res);}
  
  void addPrediction(ExecutionContext& context, double predicted, bool correct)
    {roc.addPrediction(context, predicted, correct);}
  
  virtual String toString() const
  {
    if (!roc.getSampleCount())
      return String::empty;
    
    double bestF1;
    double bestThreshold = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1);
    return T("tuned F1: ") + String(bestF1 * 100, 2) + T("% threshold = ") + String(bestThreshold);
  }
  
private:
  ROCAnalyse roc;
};

class ROCAnalysisEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictedElementsType() const
    {return probabilityType;}
  
  virtual TypePtr getRequiredSupervisionElementsType() const
    {return booleanType;}

protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new ROCAnalysisScoreObject();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<ROCAnalysisScoreObject>()->addPrediction(context, predictedObject.getDouble(), correctObject.getBoolean());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
