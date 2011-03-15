/*-----------------------------------------.---------------------------------.
| Filename: Utilities.cpp                  | Utilities for evaluators        |
| Author  : Julien Becker                  |                                 |
| Started : 23/02/2011 11:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Utilities.h"
#include <lbcpp/Core/Variable.h>
using namespace lbcpp;

/*
** BinaryClassificationConfusionMatrix
*/
BinaryClassificationConfusionMatrix::BinaryClassificationConfusionMatrix(const BinaryClassificationConfusionMatrix& other)
  : precision(0.0),
    recall(0.0),
    f1score(0.0),
    matthewsCorrelation(0.0),
    accuracy(0.0),
    truePositive(other.truePositive),
    falsePositive(other.falsePositive),
    falseNegative(other.falseNegative),
    trueNegative(other.trueNegative),
    totalCount(other.totalCount)
{
  finalize();
}

BinaryClassificationConfusionMatrix::BinaryClassificationConfusionMatrix(BinaryClassificationScore scoreToOptimize)
  : scoreToOptimize(scoreToOptimize),
    precision(0.0),
    recall(0.0),
    f1score(0.0),
    matthewsCorrelation(0.0),
    accuracy(0.0),
    truePositive(0),
    falsePositive(0),
    falseNegative(0),
    trueNegative(0),
    totalCount(0)
{
}

void BinaryClassificationConfusionMatrix::finalize()
{
  precision = computePrecision();
  recall = computeRecall();
  f1score = computeF1Score();
  matthewsCorrelation = computeMatthewsCorrelation();
  accuracy = computeAccuracy();
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
  toFixedLengthString(T("Predicted as neg.: "), 20) + toFixedLengthString(String((int)falseNegative), 15) + toFixedLengthString(String((int)trueNegative), 15) + T("\n")
  + T("ACC = ") + String(accuracy * 100.0, 2)
  + T("% P = ") + String(precision * 100.0, 2)
  + T("% R = ") + String(recall * 100.0, 2)
  + T("% F1 = ") + String(f1score * 100.0, 2)
  + T("% MCC = ") + String(matthewsCorrelation, 4) + T("\n");
}

bool BinaryClassificationConfusionMatrix::convertToBoolean(ExecutionContext& context, const Variable& variable, bool& res)
{
  if (!variable.exists())
    return false;
  
  if (variable.isBoolean())
    res = variable.getBoolean();
  else if (variable.inheritsFrom(probabilityType))
    res = variable.getDouble() > 0.5;
  else if (variable.inheritsFrom(doubleType))
    res = variable.getDouble() > 0.0; // sign
  else
  {
    context.errorCallback(T("BinaryClassificationConfusionMatrix::convertToBoolean"), T("Given type: ") + variable.getType()->toString());
    jassert(false);
    return false;
  }
  return true;
}

void BinaryClassificationConfusionMatrix::clear()
{
  truePositive = falsePositive = falseNegative = trueNegative = totalCount = 0;
}

void BinaryClassificationConfusionMatrix::set(size_t truePositive, size_t falsePositive, size_t falseNegative, size_t trueNegative)
{
  this->truePositive = truePositive;
  this->falsePositive = falsePositive;
  this->falseNegative = falseNegative;
  this->trueNegative = trueNegative;
  totalCount = truePositive + falsePositive + falseNegative + trueNegative;
}

void BinaryClassificationConfusionMatrix::addPrediction(bool predicted, bool correct, size_t count)
{
  if (predicted)
    correct ? (truePositive += count) : (falsePositive += count);
  else
    correct ? (falseNegative += count) : (trueNegative += count);
  totalCount += count;
}

void BinaryClassificationConfusionMatrix::addPredictionIfExists(ExecutionContext& context, const Variable& predicted, const Variable& correct, size_t count)
{
  bool p, c;
  if (convertToBoolean(context, predicted, p) && convertToBoolean(context, correct, c))
    addPrediction(p, c, count);
}

void BinaryClassificationConfusionMatrix::removePrediction(bool predicted, bool correct, size_t count)
{
  jassert(totalCount);
  if (predicted)
    correct ? (truePositive -= count) : (falsePositive -= count);
  else
    correct ? (falseNegative -= count) : (trueNegative -= count);
  totalCount -= count;
}

size_t BinaryClassificationConfusionMatrix::getCount(bool predicted, bool correct) const
{
  return predicted
  ? (correct ? truePositive : falsePositive)
  : (correct ? falseNegative : trueNegative);
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
  {return totalCount ? (truePositive + trueNegative) / (double)totalCount : 0.0;}

void BinaryClassificationConfusionMatrix::computePrecisionRecallAndF1(double& precision, double& recall, double& f1score) const
{
  precision = (truePositive || falsePositive) ? truePositive / (double)(truePositive + falsePositive) : 0.0;
  recall = (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;
  f1score = precision + recall > 0.0 ? (2.0 * precision * recall / (precision + recall)) : 0.0;
}

double BinaryClassificationConfusionMatrix::computeF1Score() const
{
  double denom = truePositive + (falseNegative + falsePositive) / 2.0;
  return denom ? truePositive / (double)denom : 1.0;
}

double BinaryClassificationConfusionMatrix::computePrecision() const
{return (truePositive || falsePositive) ? truePositive / (double)(truePositive + falsePositive) : 0.0;}

double BinaryClassificationConfusionMatrix::computeRecall() const
{return (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;}

bool BinaryClassificationConfusionMatrix::operator ==(const BinaryClassificationConfusionMatrix& other) const
{return truePositive == other.truePositive && falsePositive == other.falsePositive && 
  falseNegative == other.falseNegative && trueNegative == other.trueNegative && totalCount == other.totalCount;}

/*
** ROCScoreObject
*/
void ROCScoreObject::addPrediction(ExecutionContext& context, double predictedScore, bool isPositive)
{
  ScopedLock _(lock);
  isPositive ? ++numPositives : ++numNegatives;
  std::pair<size_t, size_t>& counters = predictedScores[predictedScore];
  if (isPositive)
    ++counters.second;
  else
    ++counters.first;
}

double ROCScoreObject::findBestThreshold(BinaryClassificationScore scoreToOptimize, double& bestScore) const
{
  ScoreFunction scoreFunction;
  switch (scoreToOptimize)
  {
  case binaryClassificationAccuracyScore:
    scoreFunction = &BinaryClassificationConfusionMatrix::computeAccuracy;
    break;

  case binaryClassificationF1Score:
    scoreFunction = &BinaryClassificationConfusionMatrix::computeF1Score;
    break;

  case binaryClassificationMCCScore:
    scoreFunction = &BinaryClassificationConfusionMatrix::computeMatthewsCorrelation;
    break;

  default:
    jassert(false);
    return 0.0;
  };

  return findBestThreshold(scoreFunction, bestScore);
}

double ROCScoreObject::findBestThreshold(ScoreFunction measure, double& bestScore, double margin) const
{
  ScopedLock _(lock);
  
  jassert(predictedScores.size());
  
  BinaryClassificationConfusionMatrix confusionMatrix;
  confusionMatrix.set(numPositives, numNegatives, 0, 0);
  bestScore = (confusionMatrix.*measure)(); 
  double bestThreshold = predictedScores.size() ? predictedScores.begin()->first - margin : 0.0;
  
#ifdef JUCE_DEBUG
  BinaryClassificationConfusionMatrix bestMatrix = confusionMatrix;
#endif // JUCE_DEBUG
  
  for (ScoresMap::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
  {
    if (it->second.first)
    {
      confusionMatrix.removePrediction(true, false, it->second.first);
      confusionMatrix.addPrediction(false, false, it->second.first);
    }
    if (it->second.second)
    {
      confusionMatrix.removePrediction(true, true, it->second.second);
      confusionMatrix.addPrediction(false, true, it->second.second);
    }
    jassert(confusionMatrix.getSampleCount() == numPositives + numNegatives);
    
    double result = (confusionMatrix.*measure)(); 
    if (result >= bestScore)
    {
      bestScore = result;
      bestThreshold = getBestThreshold(it, margin);
#ifdef JUCE_DEBUG
      bestMatrix = confusionMatrix;
#endif // JUCE_DEBUG
    }
  }
  
#ifdef JUCE_DEBUG
  BinaryClassificationConfusionMatrix debugMatrix;
  for (ScoresMap::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
    if (it->first > bestThreshold)
    {
      debugMatrix.addPrediction(true, false, it->second.first);
      debugMatrix.addPrediction(true, true, it->second.second);
    }
    else
    {
      debugMatrix.addPrediction(false, false, it->second.first);
      debugMatrix.addPrediction(false, true, it->second.second);
    }
  double debugScore = (debugMatrix.*measure)(); 
  jassert(debugMatrix == bestMatrix);
  jassert(debugScore == bestScore);
#endif // JUCE_DEBUG
  
  return bestThreshold;
}

double ROCScoreObject::getBestThreshold(ScoresMap::const_iterator lastLower, double margin) const
{
  ScoresMap::const_iterator nxt = lastLower;
  ++nxt;
  if (nxt == predictedScores.end())
    return lastLower->first + margin;
  else
    return (lastLower->first + nxt->first) / 2.0;
}

void ROCScoreObject::finalize()
{
  ScopedLock _(lock);

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

  bestThreshold = findBestThreshold(scoreToOptimize, bestThresholdScore);

  precision.push_back(std::make_pair(10, bestPrecAt10));
  precision.push_back(std::make_pair(25, bestPrecAt25));
  precision.push_back(std::make_pair(50, bestPrecAt50));
  precision.push_back(std::make_pair(75, bestPrecAt75));
  precision.push_back(std::make_pair(90, bestPrecAt90));
  recall.push_back(std::make_pair(10, bestRecAt10));
  recall.push_back(std::make_pair(25, bestRecAt25));
  recall.push_back(std::make_pair(50, bestRecAt50));
  recall.push_back(std::make_pair(75, bestRecAt75));
  recall.push_back(std::make_pair(90, bestRecAt90));

  predictedScores.clear();
}
