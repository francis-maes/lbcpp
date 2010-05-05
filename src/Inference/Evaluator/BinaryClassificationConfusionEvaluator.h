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

namespace lbcpp
{

class BinaryClassificationConfusionEvaluator : public Evaluator
{
public:
  BinaryClassificationConfusionEvaluator(const String& name)
    : Evaluator(name), truePositive(0), falsePositive(0), falseNegative(0), trueNegative(0), totalCount(0) {}
  BinaryClassificationConfusionEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    LabelPtr predicted = predictedObject.dynamicCast<Label>();
    LabelPtr correct = correctObject.dynamicCast<Label>();
    if (!predicted || !correct)
      return;
    jassert(predicted->getDictionary() == correct->getDictionary());
    if (predicted->getIndex() == 1)
      (correct->getIndex() == 1) ? ++truePositive : ++falsePositive;
    else
      (correct->getIndex() == 1) ? ++falseNegative : ++trueNegative;
    ++totalCount;
  }
  
  virtual String toString() const
  {
    if (!totalCount)
      return String::empty;
  
    double precision = truePositive && falsePositive ? truePositive / (double)(truePositive + falsePositive) : 0.0;
    double recall = truePositive && falseNegative ? truePositive / (double)(truePositive + falseNegative) : 0.0;
    double f1score = precision + recall > 0.0 ? (2.0 * precision * recall / (precision + recall)) : 0.0;

    return getName() + T(": TP = ") + lbcpp::toString(truePositive) + T(" FP = ") + lbcpp::toString(falsePositive)
                     + T(": FN = ") + lbcpp::toString(falseNegative) + T(" TN = ") + lbcpp::toString(trueNegative) + T("\n")
                     + T("\tP = ") + String(precision * 100.0, 2)
                     + T("% R = ") + String(recall * 100.0, 2)
                     + T("% F1 = ") + String(f1score * 100.0, 2)
                     + T("% MCC = ") + String(computeMatthewsCorrelation(), 4);
  }

  virtual double getDefaultScore() const
    {return computeMatthewsCorrelation();}


protected:
 // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;

  double computeMatthewsCorrelation() const
  {
    size_t positiveCount = truePositive + falseNegative;
    size_t negativeCount = falsePositive + trueNegative;

    size_t predictedPositiveCount = truePositive + falsePositive;
    size_t predictedNegativeCount = falseNegative + trueNegative;

    double mccNo = (double)(truePositive * trueNegative) - (double)(falsePositive * falseNegative);
    double mccDeno = (double)positiveCount * (double)negativeCount * (double)predictedPositiveCount * (double)predictedNegativeCount;
    return mccDeno ? (mccNo / sqrt(mccDeno)) : mccNo;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
