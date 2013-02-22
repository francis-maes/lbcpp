/*-----------------------------------------.---------------------------------.
| Filename: Utilities.cpp                  | Utilities for evaluators        |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2013 11:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Utilities.h"
#include <lbcpp/Core/XmlSerialisation.h>

using namespace lbcpp;

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

/*
** BinaryClassificationConfusionMatrix
*/
BinaryClassificationConfusionMatrix::BinaryClassificationConfusionMatrix(
                    BinaryClassificationScore scoreToMinimize, double threshold)
  : scoreToMinimize(scoreToMinimize), threshold(threshold),
    truePositive(0), falsePositive(0),
    falseNegative(0), trueNegative(0),
    totalCount(0)
  {
    setName(binaryClassificationScoreEnumeration->getElementName(scoreToMinimize)
           + T(" @ ") + String(threshold, 3));
  }

double BinaryClassificationConfusionMatrix::getScoreToMinimize() const
{
  switch (scoreToMinimize)
  {
    case binaryClassificationAccuracyScore:
    case binaryClassificationAreaUnderCurve:
      return 1.0 - computeAccuracy();
    case binaryClassificationF1Score:
      return 1.0 - computeF1Score();
    case binaryClassificationMCCScore:
      return 1.0 - computeMatthewsCorrelation();
    case binaryClassificationSensitivityAndSpecificityScore:
      return 1.0 - computeSensitivityAndSpecificity();
    default:
      jassertfalse;
  }
  return DBL_MAX;
}

String BinaryClassificationConfusionMatrix::toString() const
{
  return toFixedLengthString(T("Actual value: "), 20) + toFixedLengthString(T("positive"), 15) + toFixedLengthString(T("negative"), 15) + T("\n")
         + toFixedLengthString(T("Predicted as pos.: "), 20) + toFixedLengthString(String((int)truePositive), 15) + toFixedLengthString(String((int)falsePositive), 15) + T("\n")
         + toFixedLengthString(T("Predicted as neg.: "), 20) + toFixedLengthString(String((int)falseNegative), 15) + toFixedLengthString(String((int)trueNegative), 15) + T("\n")
         + T("Accuracy @ ") + String(threshold, 2) + (" = ") + String(computeAccuracy() * 100.0, 2) + T("%")
         + T("Precision @ ") + String(threshold, 2) + (" = ") + String(computePrecision() * 100.0, 2) + T("%")
         + T("Recall @ ") + String(threshold, 2) + (" = ") + String(computeRecall() * 100.0, 2) + T("%")
         + T("F1 @ ") + String(threshold, 2) + (" = ") + String(computeF1Score() * 100.0, 2) + T("%")
         + T("MCC @ ") + String(threshold, 2) + (" = ") + String(computeMatthewsCorrelation(), 4) + T("\n");
}

void BinaryClassificationConfusionMatrix::addPredictionIfExists(const Variable& predicted, const Variable& correct)
{
  if (!correct.exists())
    return;
  jassert(predicted.exists());
  jassert(correct.inheritsFrom(sumType(booleanType, probabilityType)));
  jassert(predicted.inheritsFrom(sumType(booleanType, probabilityType)));
  
  const bool predictedValue = predicted.isBoolean() ? predicted.getBoolean() : predicted.getDouble() >= threshold;
  const bool correctValue = correct.isBoolean() ? correct.getBoolean() : correct.getDouble() > 0.5f;
  addPrediction(predictedValue, correctValue);
}

void BinaryClassificationConfusionMatrix::addPrediction(bool predicted, bool correct)
{
  if (predicted)
    correct ? ++truePositive : ++falsePositive;
  else
    correct ? ++falseNegative : ++trueNegative;
  ++totalCount;
}


double BinaryClassificationConfusionMatrix::computeAccuracy() const
{
  return totalCount ? (truePositive + trueNegative) / (double)totalCount : 0.0;
}

double BinaryClassificationConfusionMatrix::computeF1Score() const
{
  double denom = truePositive + (falseNegative + falsePositive) / 2.0;
  return denom ? truePositive / denom : 1.0;
}

double BinaryClassificationConfusionMatrix::computePrecision() const
{
  return (truePositive || falsePositive) ? truePositive / (double)(truePositive + falsePositive) : 0.0;
}

double BinaryClassificationConfusionMatrix::computeRecall() const
{
  return (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;
}

double BinaryClassificationConfusionMatrix::computeSpecificity() const
{
  return (trueNegative || falsePositive) ? trueNegative / (double)(trueNegative + falsePositive) : 0.0;
}

double BinaryClassificationConfusionMatrix::computeMatthewsCorrelation() const
{
  size_t predictedPositiveCount = truePositive + falsePositive;
  size_t predictedNegativeCount = falseNegative + trueNegative;
  
  double mccNo = (truePositive * trueNegative) - (falsePositive * falseNegative);
  double mccDeno = getPositives() * getNegatives() * predictedPositiveCount * predictedNegativeCount;
  return mccDeno ? (mccNo / sqrt(mccDeno)) : 0.f;
}

double BinaryClassificationConfusionMatrix::computeSensitivityAndSpecificity() const
{
  const double sensitivity = computeSensitivity();
  const double specificity = computeSpecificity();
  const double sum = sensitivity + specificity;
  if (sum < 10e-6)
    return 0.0;
  return 2 * (sensitivity * specificity) / sum;
}

void BinaryClassificationConfusionMatrix::saveToXml(XmlExporter& exporter) const
{
  ScoreObject::saveToXml(exporter);
  exporter.enter(T("values"));
  String res = String((int)truePositive) + T(" ")
             + String((int)falsePositive) + T(" ")
             + String((int)falseNegative) + T(" ")
             + String((int)trueNegative);
  exporter.addTextElement(res);
  exporter.leave();
}

bool BinaryClassificationConfusionMatrix::loadFromXml(XmlImporter& importer)
{
  ScoreObject::loadFromXml(importer);

  importer.enter(T("values"));
  StringArray tokens;
  tokens.addTokens(importer.getAllSubText(), true);
  if (tokens.size() != 4)
    return false;
  
  Variable v = Variable::createFromString(importer.getContext(), positiveIntegerType, tokens[0]);
  if (!v.exists())
    return false;
  truePositive = v.getInteger();
  
  v = Variable::createFromString(importer.getContext(), positiveIntegerType, tokens[1]);
  if (!v.exists())
    return false;
  falsePositive = v.getInteger();
  
  v = Variable::createFromString(importer.getContext(), positiveIntegerType, tokens[2]);
  if (!v.exists())
    return false;
  falseNegative = v.getInteger();
  
  v = Variable::createFromString(importer.getContext(), positiveIntegerType, tokens[3]);
  if (!v.exists())
    return false;
  trueNegative = v.getInteger();
  
  totalCount = truePositive + falsePositive + falseNegative + trueNegative;
  importer.leave();

  return true;
}

/*
** BinaryClassificationCurveScoreObject
*/
BinaryClassificationCurveScoreObject::BinaryClassificationCurveScoreObject(BinaryClassificationScore scoreToMinimize)
: scoreToMinimize(scoreToMinimize), areaUnderCurve(0.f), accuracyAt5Fpr(0.f)
{
  thresholds[0.f] = true;
  thresholds[1.f] = true;
}

double BinaryClassificationCurveScoreObject::getScoreToMinimize() const
{
  if (!bestConfusionMatrix)
    return DBL_MAX;
  if (scoreToMinimize == binaryClassificationAreaUnderCurve)
    return 1.f - areaUnderCurve;
  return bestConfusionMatrix->getScoreToMinimize();
}

void BinaryClassificationCurveScoreObject::addPrediction(const Variable& predicted, const Variable& correct)
{
  predictions.push_back(std::make_pair(predicted, correct));
  if (predicted.isDouble())
    thresholds[predicted.getDouble()] = true;
}

void BinaryClassificationCurveScoreObject::finalize(bool saveConfusionMatrices)
{
  // Create one confusion matrix per threshold
  confusionMatrices.clear();
  confusionMatrices.reserve(thresholds.size());
  for (std::map<double, bool>::iterator it = thresholds.begin();
       it != thresholds.end(); ++it)
    confusionMatrices.push_back(createBinaryConfusionMatrix(it->first));
  // Determine the best confusion matrix according to the score to minimize
  double bestScore = DBL_MAX;
  for (size_t i = 0; i < confusionMatrices.size(); ++i)
    if (confusionMatrices[i]->getScoreToMinimize() < bestScore)
    {
      bestConfusionMatrix = confusionMatrices[i];
      bestScore = confusionMatrices[i]->getScoreToMinimize();
    }
  // Compute the area under the curve
  computeAreaUnderCurve();

  // Compute Accuracy at 5% FPR
  computeAccuracyAt5Fpr();

  if (!saveConfusionMatrices)
    confusionMatrices.clear();
}

ContainerPtr BinaryClassificationCurveScoreObject::createBinaryClassificationCurveElements() const
{
  VectorPtr res = vector(binaryClassificationCurveElementClass, confusionMatrices.size());
  for (size_t i = 0; i < confusionMatrices.size(); ++i)
    res->setElement(i, new BinaryClassificationCurveElement(confusionMatrices[i]));
  return res;
}

void BinaryClassificationCurveScoreObject::getAllThresholds(std::vector<double>& result) const
{
  result.clear();
  for (std::map<double, bool>::const_iterator it = thresholds.begin();
       it != thresholds.end(); ++it)
    result.push_back(it->first);
}

BinaryClassificationConfusionMatrixPtr BinaryClassificationCurveScoreObject::createBinaryConfusionMatrix(double threshold) const
{
  BinaryClassificationConfusionMatrixPtr res = new BinaryClassificationConfusionMatrix(scoreToMinimize, threshold);
  const size_t n = predictions.size();
  for (size_t i = 0; i < n; ++i)
    res->addPredictionIfExists(predictions[i].first, predictions[i].second);
  return res;
}

void BinaryClassificationCurveScoreObject::computeAreaUnderCurve()
{
  std::map<double, double> falsePositiveRates;
  falsePositiveRates[1.f] = 1.f;

  for (size_t i = 0; i < confusionMatrices.size(); ++i)
    falsePositiveRates[1.f - confusionMatrices[i]->computeSpecificity()] = confusionMatrices[i]->computeSensitivity();

  areaUnderCurve = 0.f;
  accuracyAt5Fpr = 0.f;
  double prevFpr = 0.f;
  double prevTpr = 0.f;
  for (std::map<double, double>::iterator it = falsePositiveRates.begin();
       it != falsePositiveRates.end(); ++it)
  {
    const double width = it->first - prevFpr;
    const double height = it->second + prevTpr;
    // Compute Area Under Curve
    areaUnderCurve += width * height / 2;
    prevFpr = it->first;
    prevTpr = it->second;
  }
}

void BinaryClassificationCurveScoreObject::computeAccuracyAt5Fpr()
{
  std::map<double, double> falsePositiveRates;
  falsePositiveRates[1.f] = 1.f;
  
  for (size_t i = 0; i < confusionMatrices.size(); ++i)
    falsePositiveRates[1.f - confusionMatrices[i]->computeSpecificity()] = confusionMatrices[i]->computeAccuracy();

  accuracyAt5Fpr = 0.f;
  double prevFpr = 0.f;
  double prevAccuracy = 0.f;
  for (std::map<double, double>::iterator it = falsePositiveRates.begin();
       it != falsePositiveRates.end(); ++it)
  {
    if (it->first >= 0.05f)
    {
      const double width = it->first - prevFpr;
      const double deltaHeight = it->second - prevAccuracy;
      // Compute Accuracy at 5% FPR
      accuracyAt5Fpr = prevAccuracy + deltaHeight / width * (0.05f - prevFpr);
      break;
    }
    prevFpr = it->first;
    prevAccuracy = it->second;
  }
}

