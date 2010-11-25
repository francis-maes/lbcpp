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

  static bool convertToBoolean(ExecutionContext& context, const Variable& variable, bool& res)
  {
    if (!variable.exists())
      return false;

    if (variable.isBoolean())
      res = variable.getBoolean();
    else if (variable.inheritsFrom(probabilityType))
      res = variable.getDouble() > 0.5;
    else
    {
      context.errorCallback(T("BinaryClassificationConfusionEvaluator::convertToBoolean"), T("Given type: ") + variable.getType()->toString());
      jassert(false);
      return false;
    }
    return true;
  }

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject)
  {
    bool predicted, correct;
    if (convertToBoolean(context, predictedObject, predicted) && convertToBoolean(context, correctObject, correct))
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
    res.push_back(std::make_pair(T("Precision"), precision));
    res.push_back(std::make_pair(T("Recall"), recall));
    res.push_back(std::make_pair(T("F1"), f1));
    res.push_back(std::make_pair(T("MCC"), mcc));
    res.push_back(std::make_pair(T("Accuracy"), accuracy));
  }

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

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct)
  {
    if (!predicted.exists() || !correct.exists())
      return;
    jassert(predicted.isDouble() && correct.isBoolean());
    roc.addPrediction(context, predicted.getDouble(), correct.getBoolean());
  }
 
  virtual String toString() const
  {
    if (!roc.getSampleCount())
      return String::empty;

    double bestF1;
    double bestThreshold = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1);
    return T("tuned F1: ") + String(bestF1 * 100, 2) + T("% threshold = ") + String(bestThreshold);
  }

  virtual double getDefaultScore() const
  {
    double bestF1;
    roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1);
    return bestF1;
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {roc.getScores(res);}

private:
  ROCAnalyse roc;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
