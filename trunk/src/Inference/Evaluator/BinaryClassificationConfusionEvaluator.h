/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationConfusio..h| Binary Classification           |
| Author  : Francis Maes                   |   Confusion Matrix Evaluator    |
| Started : 27/04/2010 16:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
# define LBCPP_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_

# include <lbcpp/Inference/Evaluator.h>
# include <fstream>

namespace lbcpp
{

class BinaryClassificationConfusionEvaluator : public Evaluator
{
public:
  BinaryClassificationConfusionEvaluator(const String& name)
    : Evaluator(name) {}
  BinaryClassificationConfusionEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    LabelPtr predicted = predictedObject.dynamicCast<Label>();
    LabelPtr correct = correctObject.dynamicCast<Label>();
    if (!predicted || !correct)
      return;
    jassert(predicted->getDictionary() == correct->getDictionary());
    confusionMatrix.addPrediction(predicted->getIndex() == 1, correct->getIndex() == 1);
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

    return getName() + T("\n") + confusionMatrix.toString()
                     + T("P = ") + String(precision * 100.0, 2)
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

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ScalarPtr predicted = predictedObject.dynamicCast<Scalar>();
    LabelPtr correct = correctObject.dynamicCast<Label>();
    if (!predicted || !correct)
      return;
    roc.addPrediction(predicted->getValue(), correct->getIndex() == 1);
  }
 
  virtual String toString() const
  {
    if (!roc.getSampleCount())
      return String::empty;

    double bestF1;
    double bestThreshold = roc.findThresholdMaximisingF1(bestF1);

    return T("tuned F1: ") + String(bestF1 * 100, 2) + T("% threshold = ") + lbcpp::toString(bestThreshold);
  }

  virtual double getDefaultScore() const
  {
    double bestF1;
    roc.findThresholdMaximisingF1(bestF1);
    return bestF1;
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {roc.getScores(res);}

private:
  ROCAnalyse roc;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
