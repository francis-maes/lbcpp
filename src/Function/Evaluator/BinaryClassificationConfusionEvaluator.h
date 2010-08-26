/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationConfusio..h| Binary Classification           |
| Author  : Francis Maes                   |   Confusion Matrix Evaluator    |
| Started : 27/04/2010 16:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
# define LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class BinaryClassificationConfusionEvaluator : public Evaluator
{
public:
  BinaryClassificationConfusionEvaluator(const String& name)
    : Evaluator(name) {}
  BinaryClassificationConfusionEvaluator() {}

  static bool convertToBoolean(const Variable& variable, bool& res)
  {
    if (!variable)
      return false;

    if (variable.isBoolean())
      res = variable.getBoolean();
    else if (variable.inheritsFrom(probabilityType()))
      res = variable.getDouble() > 0.5;
    else
    {
      jassert(false);
      return false;
    }
    return true;
  }

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    bool predicted, correct;
    if (convertToBoolean(predictedObject, predicted) && convertToBoolean(correctObject, correct))
      confusionMatrix.addPrediction(predicted, correct);
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
  }

  virtual String toString() const
  {
    if (!confusionMatrix.getSampleCount())
      return String::empty;
  
    double precision, recall, f1score;
    confusionMatrix.computePrecisionRecallAndF1(precision, recall, f1score);

    double accuracy = confusionMatrix.computeAccuracy();

    return getName() + T("\n") + confusionMatrix.toString()
                     + T("ACC = ") + String(accuracy * 100.0, 2)
                     + T("% P = ") + String(precision * 100.0, 2)
                     + T("% R = ") + String(recall * 100.0, 2)
                     + T("% F1 = ") + String(f1score * 100.0, 2)
                     + T("% MCC = ") + String(confusionMatrix.computeMatthewsCorrelation(), 4);
  }

  virtual double getDefaultScore() const
  {
    double precision, recall, f1score;
    confusionMatrix.computePrecisionRecallAndF1(precision, recall, f1score);
    return f1score;
  }

protected:
  BinaryClassificationConfusionMatrix confusionMatrix;
};

class ROCAnalysisEvaluator : public Evaluator
{
public:
  ROCAnalysisEvaluator(const String& name)
    : Evaluator(name)  {}
  ROCAnalysisEvaluator() {}

  virtual void addPrediction(const Variable& predicted, const Variable& correct)
  {
    if (!predicted || !correct)
      return;
    jassert(predicted.isDouble() && correct.isBoolean());
    roc.addPrediction(predicted.getDouble(), correct.getBoolean());
  }
 
  virtual String toString() const
  {
    if (!roc.getSampleCount())
      return String::empty;

    double bestF1, precision, recall;
    double bestThreshold = roc.findThresholdMaximisingF1(bestF1, precision, recall);

    return T("tuned F1: ") + String(bestF1 * 100, 2) + T("% prec = ") + String(precision * 100, 2) +
            T("% recall = ") + String(recall * 100, 2) + T("% threshold = ") + String(bestThreshold);
  }

  virtual double getDefaultScore() const
  {
    double bestF1, precision, recall;
    roc.findThresholdMaximisingF1(bestF1, precision, recall);
    return bestF1;
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {roc.getScores(res);}

private:
  ROCAnalyse roc;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
