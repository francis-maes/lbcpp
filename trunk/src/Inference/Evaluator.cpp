/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Evaluator.h>

#include "Evaluator/ObjectContainerEvaluator.h"
#include "Evaluator/ClassificationAccuracyEvaluator.h"
#include "Evaluator/BinaryClassificationConfusionEvaluator.h"
#include "Evaluator/RegressionErrorEvaluator.h"
using namespace lbcpp;

EvaluatorPtr lbcpp::classificationAccuracyEvaluator(const String& name)
  {return new ClassificationAccuracyEvaluator(name);}

EvaluatorPtr lbcpp::binaryClassificationConfusionEvaluator(const String& name)
  {return new BinaryClassificationConfusionEvaluator(name);}

EvaluatorPtr lbcpp::rocAnalysisEvaluator(const String& name)
  {return new ROCAnalysisEvaluator(name);}

EvaluatorPtr lbcpp::regressionErrorEvaluator(const String& name)
  {return new RegressionErrorEvaluator(name);}

EvaluatorPtr lbcpp::dihedralRegressionErrorEvaluator(const String& name)
  {return new DihedralAngleRegressionErrorEvaluator(name);}

EvaluatorPtr lbcpp::objectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator)
  {return new ObjectContainerEvaluator(name, objectEvaluator);}


/*
** RegressionErrorEvaluator
*/
RegressionErrorEvaluator::RegressionErrorEvaluator(const String& name)
  : Evaluator(name), absoluteError(new ScalarVariableMean()), squaredError(new ScalarVariableMean())
{
}
  
void RegressionErrorEvaluator::addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
{
  ScalarPtr predicted = predictedObject.dynamicCast<Scalar>();
  ScalarPtr correct = correctObject.dynamicCast<Scalar>();
  if (predicted && correct)
    addDelta(predicted->getValue() - correct->getValue());
}

void RegressionErrorEvaluator::addDelta(double delta)
{
  absoluteError->push(fabs(delta));
  squaredError->push(delta * delta);
}

String RegressionErrorEvaluator::toString() const
{
  double count = squaredError->getCount();
  if (!count)
    return String::empty;
  return getName() + T(": rmse = ") + String(getRMSE(), 4)
      + T(" abs = ") + String(absoluteError->getMean(), 4)
      + T(" (") + lbcpp::toString(count) + T(" examples)");
}

double RegressionErrorEvaluator::getRMSE() const
{
  return sqrt(squaredError->getMean());
}

/*
** BinaryClassificationConfusionMatrix
*/

BinaryClassificationConfusionMatrix::BinaryClassificationConfusionMatrix()
  : truePositive(0), falsePositive(0), falseNegative(0), trueNegative(0), totalCount(0)
{
}

inline String toFixedLengthString(const String& str, int size)
{
  jassert(str.length() <= size);
  String res = str;
  while (res.length() < size)
    if (res.length() % 2 == 0)
      res = T(" ") + res;
    else
      res += T(" ");
  return res;
}

String BinaryClassificationConfusionMatrix::toString() const
{
  return toFixedLengthString(T("Actual value: "), 20) + toFixedLengthString(T("positive"), 15) + toFixedLengthString(T("negative"), 15) + T("\n") +
         toFixedLengthString(T("Predicted as pos.: "), 20) + toFixedLengthString(String((int)truePositive), 15) + toFixedLengthString(String((int)falsePositive), 15) + T("\n") +
         toFixedLengthString(T("Predicted as neg.: "), 20) + toFixedLengthString(String((int)falseNegative), 15) + toFixedLengthString(String((int)trueNegative), 15) + T("\n");
}

void BinaryClassificationConfusionMatrix::clear()
{
  truePositive = falsePositive = falseNegative = trueNegative = totalCount = 0;
}

void BinaryClassificationConfusionMatrix::addPrediction(bool predicted, bool correct)
{
  if (predicted)
    correct ? ++truePositive : ++falsePositive;
  else
    correct ? ++falseNegative : ++trueNegative;
  ++totalCount;
}

double BinaryClassificationConfusionMatrix::computeMatthewsCorrelation() const
{
  size_t positiveCount = truePositive + falseNegative;
  size_t negativeCount = falsePositive + trueNegative;

  size_t predictedPositiveCount = truePositive + falsePositive;
  size_t predictedNegativeCount = falseNegative + trueNegative;

  double mccNo = (double)(truePositive * trueNegative) - (double)(falsePositive * falseNegative);
  double mccDeno = (double)positiveCount * (double)negativeCount * (double)predictedPositiveCount * (double)predictedNegativeCount;
  return mccDeno ? (mccNo / sqrt(mccDeno)) : mccNo;
}

void BinaryClassificationConfusionMatrix::computePrecisionRecallAndF1(double& precision, double& recall, double& f1score) const
{
  precision = (truePositive || falsePositive) ? truePositive / (double)(truePositive + falsePositive) : 0.0;
  recall = (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;
  f1score = precision + recall > 0.0 ? (2.0 * precision * recall / (precision + recall)) : 0.0;
}

/*
** ROCAnalyse
*/
void ROCAnalyse::addPrediction(double predictedScore, bool isPositive)
{
  isPositive ? ++numPositives : ++numNegatives;
  predictedScores.insert(std::make_pair(predictedScore, isPositive));
}

double ROCAnalyse::findBestThreshold(double& bestF1Score) const
{
  size_t truePositives = numPositives;
  size_t falsePositives = numNegatives;

  bestF1Score = 0.0;
  double bestThreshold = 0.5;
  jassert(predictedScores.size() == (numPositives + numNegatives));
  for (std::multimap<double, bool>::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
  {
    size_t falseNegatives = numPositives - truePositives;
    double f1 = 2.0 * truePositives / (2.0 * truePositives + falseNegatives + falsePositives);
    if (f1 > bestF1Score)
    {
      bestF1Score = f1;
      bestThreshold = it->first;
    }
    if (it->second)
      --truePositives;
    else
      --falsePositives;
  }
  return bestThreshold;
}
