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

class ROCAnalysisEvaluator : public Evaluator
{
public:
  ROCAnalysisEvaluator(const String& name)
    : Evaluator(name), numPositives(0), numNegatives(0) {}
  ROCAnalysisEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ScalarPtr predicted = predictedObject.dynamicCast<Scalar>();
    LabelPtr correct = correctObject.dynamicCast<Label>();
    if (!predicted || !correct)
      return;
    bool isPositive = (correct->getIndex() == 1);
    isPositive ? ++numPositives : ++numNegatives;
    predictedScores.insert(std::make_pair(predicted->getValue(), isPositive));
  }
 
  virtual String toString() const
  {
    String res;

    size_t truePositives = numPositives;
    size_t falsePositives = numNegatives;

/*    File directory(T("C:\\Projets\\LBC++\\projects\\temp"));
    static int count = 0;
    File curveFile = directory.getChildFile(T("Curve") + String(count) + T(".txt"));
    curveFile.deleteFile();
    std::ofstream ostr((const char* )curveFile.getFullPathName());
    ++count;*/

    double bestF1 = 0.0;
    double bestThreshold = 0.5;
    jassert(predictedScores.size() == (numPositives + numNegatives));
    for (std::multimap<double, bool>::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
    {
      size_t falseNegatives = numPositives - truePositives;
      double f1 = 2.0 * truePositives / (2.0 * truePositives + falseNegatives + falsePositives);
      if (f1 > bestF1)
        bestF1 = f1, bestThreshold = it->first;
      //ostr << it->first << " " << lbcpp::toString(100 * falsePositives / (double)numNegatives) << " " 
      //      << lbcpp::toString(100 * truePositives / (double)numPositives) << " " << (100.0 * f1) << std::endl;
      if (it->second)
        --truePositives;
      else
        --falsePositives;
    }
    jassert(truePositives == 0 && falsePositives == 0);
    return T("Best F1: ") + String(bestF1 * 100, 2) + T("% threshold = ") + lbcpp::toString(bestThreshold);
  }

  virtual double getDefaultScore() const
  {
    return 0.0;
  }
 
private:
  std::multimap<double, bool> predictedScores;
  size_t numPositives, numNegatives;
};

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

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_BINARY_CLASSIFICATION_CONFUSION_H_
