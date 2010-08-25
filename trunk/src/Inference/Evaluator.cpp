/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Evaluator.h>

#include "Evaluator/ContainerElementsEvaluator.h"
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

EvaluatorPtr lbcpp::containerElementsEvaluator(const String& name, EvaluatorPtr elementEvaluator)
  {return new ContainerElementsEvaluator(name, elementEvaluator);}


/*
** RegressionErrorEvaluator
*/
RegressionErrorEvaluator::RegressionErrorEvaluator(const String& name)
  : Evaluator(name), absoluteError(new ScalarVariableMean()), squaredError(new ScalarVariableMean())
{
}
  
void RegressionErrorEvaluator::addPrediction(const Variable& predicted, const Variable& correct)
{
  if (predicted && correct)
    addDelta(predicted.getDouble() - correct.getDouble());
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
      + T(" (") + String((int)count) + T(" examples)");
}

void RegressionErrorEvaluator::getScores(std::vector< std::pair<String, double> >& res) const
{
  res.push_back(std::make_pair(T("RMSE"), -getRMSE()));
  res.push_back(std::make_pair(T("AbsE"), -absoluteError->getMean()));
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

double BinaryClassificationConfusionMatrix::computeAccuracy() const
  {return (truePositive + trueNegative) / (double)totalCount;}

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
  std::pair<size_t, size_t>& counters = predictedScores[predictedScore];
  if (isPositive)
    ++counters.second;
  else
    ++counters.first;
}

double ROCAnalyse::findThresholdMaximisingF1(double& bestF1Score, double& precision, double& recall) const
{
  size_t truePositives = numPositives;
  size_t falsePositives = numNegatives;

  bestF1Score = -DBL_MAX;
  double bestThreshold = 0.0;
  //std::cout << "=========" << std::endl;
  for (std::map<double, std::pair<size_t, size_t> >::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
  {
    size_t falseNegatives = numPositives - truePositives;
    double f1 = 2.0 * truePositives / (2.0 * truePositives + falseNegatives + falsePositives);
    //std::cout << "(x >= " << it->first << ") ==> prec = " << precision << " recall = " << recall << " f1 = " << f1 << std::endl;
    if (f1 > bestF1Score)
    {
      bestF1Score = f1;
      bestThreshold = it->first;
      recall = truePositives / (double)numPositives;
      precision = truePositives / (double)(truePositives + falsePositives);
    }
    falsePositives -= it->second.first;
    truePositives -= it->second.second;
  }
  return bestThreshold;
}

double ROCAnalyse::findThresholdMaximisingRecallGivenPrecision(double minimumPrecision, double& bestRecall) const
{
  size_t truePositives = numPositives;
  size_t falsePositives = numNegatives;

  bestRecall = 0.0;
  double bestThreshold = 0.0;
  //std::cout << "=========" << std::endl;
  for (std::map<double, std::pair<size_t, size_t> >::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
  {
    double recall = truePositives / (double)numPositives;
    double precision = truePositives / (double)(truePositives + falsePositives);
    if (precision >= minimumPrecision)
    {
      if (recall > bestRecall)
        bestRecall = recall, bestThreshold = it->first;
    }
    falsePositives -= it->second.first;
    truePositives -= it->second.second;
  }
  return bestThreshold;
}

void ROCAnalyse::getScores(std::vector< std::pair<String, double> >& res) const
{
  size_t truePositives = numPositives;
  size_t falsePositives = numNegatives;

  double bestF1Score = 0.0;
  double bestPrecAt10 = 0.0, bestPrecAt25 = 0.0, bestPrecAt50 = 0.0, bestPrecAt75 = 0.0, bestPrecAt90 = 0.0;
  double bestRecAt10 = 0.0, bestRecAt25 = 0.0, bestRecAt50 = 0.0, bestRecAt75 = 0.0, bestRecAt90 = 0.0;

  for (std::map<double, std::pair<size_t, size_t> >::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
  {
    size_t falseNegatives = numPositives - truePositives;
    double recall = truePositives / (double)numPositives;
    double precision = truePositives / (double)(truePositives + falsePositives);
    double f1 = 2.0 * truePositives / (2.0 * truePositives + falseNegatives + falsePositives);

    bestF1Score = juce::jmax(f1, bestF1Score);
    
    if (recall >= 0.1)
      bestPrecAt10 = juce::jmax(bestPrecAt10, precision);
    if (recall >= 0.25)
      bestPrecAt25 = juce::jmax(bestPrecAt25, precision);
    if (recall >= 0.5)
      bestPrecAt50 = juce::jmax(bestPrecAt50, precision);
    if (recall >= 0.75)
      bestPrecAt75 = juce::jmax(bestPrecAt75, precision);
    if (recall >= 0.9)
      bestPrecAt90 = juce::jmax(bestPrecAt90, precision);
    
    if (precision >= 0.1)
      bestRecAt10 = juce::jmax(bestRecAt10, recall);
    if (precision >= 0.25)
      bestRecAt25 = juce::jmax(bestRecAt25, recall);
    if (precision >= 0.5)
      bestRecAt50 = juce::jmax(bestRecAt50, recall);
    if (precision >= 0.75)
      bestRecAt75 = juce::jmax(bestRecAt75, recall);
    if (precision >= 0.9)
      bestRecAt90 = juce::jmax(bestRecAt90, recall);

    falsePositives -= it->second.first;
    truePositives -= it->second.second;
  }

  res.push_back(std::make_pair(T("F1"), bestF1Score));
  res.push_back(std::make_pair(T("Prec@10"), bestPrecAt10));
  res.push_back(std::make_pair(T("Prec@25"), bestPrecAt25));
  res.push_back(std::make_pair(T("Prec@50"), bestPrecAt50));
  res.push_back(std::make_pair(T("Prec@75"), bestPrecAt75));
  res.push_back(std::make_pair(T("Prec@90"), bestPrecAt90));
  res.push_back(std::make_pair(T("Rec@10"), bestRecAt10));
  res.push_back(std::make_pair(T("Rec@25"), bestRecAt25));
  res.push_back(std::make_pair(T("Rec@50"), bestRecAt50));
  res.push_back(std::make_pair(T("Rec@75"), bestRecAt75));
  res.push_back(std::make_pair(T("Rec@90"), bestRecAt90));
}
