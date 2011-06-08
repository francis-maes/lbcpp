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
#include <lbcpp/Core/XmlSerialisation.h>

using namespace lbcpp;

/*
** BinaryClassificationConfusionMatrix
*/
BinaryClassificationConfusionMatrix::BinaryClassificationConfusionMatrix(const BinaryClassificationConfusionMatrix& other)
  : scoreToOptimize(other.scoreToOptimize),
    truePositive(other.truePositive),
    falsePositive(other.falsePositive),
    falseNegative(other.falseNegative),
    trueNegative(other.trueNegative),
    totalCount(other.totalCount)
{
}

BinaryClassificationConfusionMatrix::BinaryClassificationConfusionMatrix(BinaryClassificationScore scoreToOptimize)
  : scoreToOptimize(scoreToOptimize),
    truePositive(0),
    falsePositive(0),
    falseNegative(0),
    trueNegative(0),
    totalCount(0)
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
    toFixedLengthString(T("Predicted as neg.: "), 20) + toFixedLengthString(String((int)falseNegative), 15) + toFixedLengthString(String((int)trueNegative), 15) + T("\n")
    + T("ACC = ") + String(computeAccuracy() * 100.0, 2)
    + T("% P = ") + String(computePrecision() * 100.0, 2)
    + T("% R = ") + String(computeRecall() * 100.0, 2)
    + T("% F1 = ") + String(computeF1Score() * 100.0, 2)
    + T("% MCC = ") + String(computeMatthewsCorrelation(), 4) + T("\n");
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

double BinaryClassificationConfusionMatrix::computeSensitivity() const
  {return (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;}

double BinaryClassificationConfusionMatrix::computeSpecificity() const
  {return (trueNegative || falsePositive) ? trueNegative / (double)(trueNegative + falsePositive) : 0.0;}

double BinaryClassificationConfusionMatrix::computeSensitivityAndSpecificity() const
{
  const double sensitivity = computeSensitivity();
  const double specificity = computeSpecificity();
  const double sum = sensitivity + specificity;
  if (sum < 10e-6)
    return 0.0;
  return 2 * (sensitivity * specificity) / sum;
  //return (computeSensitivity() + computeSpecificity() / 2;
}

void BinaryClassificationConfusionMatrix::saveToXml(XmlExporter& exporter) const
{
  String res = String((int)truePositive) + T(" ")
             + String((int)falsePositive) + T(" ")
             + String((int)falseNegative) + T(" ")
             + String((int)trueNegative);
  exporter.addTextElement(res);
}

bool BinaryClassificationConfusionMatrix::loadFromXml(XmlImporter& importer)
{
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
  return true;
}

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
  
  case binaryClassificationSensitivityAndSpecificityScore:
    scoreFunction = &BinaryClassificationConfusionMatrix::computeSensitivityAndSpecificity;
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
  
  //jassert(predictedScores.size());
  
  BinaryClassificationConfusionMatrix confusionMatrix;
  confusionMatrix.set(numPositives, numNegatives, 0, 0);
  bestScore = (confusionMatrix.*measure)(); 
  double bestThreshold = predictedScores.size() ? predictedScores.begin()->first - margin : 0.0;
  
  BinaryClassificationConfusionMatrix bestMatrix = confusionMatrix;  
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
      bestMatrix = confusionMatrix;
    }
  }

  const_cast<ROCScoreObject*>(this)->bestConfusionMatrix = new BinaryClassificationConfusionMatrix(bestMatrix);
  const_cast<ROCScoreObject*>(this)->bestThreshold = bestThreshold;
  bestConfusionMatrix->setName(getName() + T(" confusion matrix"));

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

BinaryClassificationConfusionMatrixPtr ROCScoreObject::findBestSensitivitySpecificityTradeOff() const
{
  const size_t n = confusionMatrices.size();
  if (n == 0)
    return BinaryClassificationConfusionMatrixPtr();

  double bestDistance = DBL_MAX;
  size_t bestIndex = (size_t)-1;
  for (size_t i = 0; i < n; ++i)
  {
    double sensitivity = 1 - confusionMatrices[i]->computeRecall();
    double specificity = 1 - confusionMatrices[i]->computeSpecificity();
    sensitivity *= sensitivity;
    specificity *= specificity;
    const double euclideanDistance = sqrt(sensitivity + specificity);
    if (euclideanDistance < bestDistance)
    {
      bestDistance = euclideanDistance;
      bestIndex = i;
    }
  }
  jassert(bestIndex != (size_t)-1);
  return confusionMatrices[bestIndex];
}

void ROCScoreObject::finalize(bool saveConfusionMatrices)
{
  ScopedLock _(lock);

  bestThreshold = findBestThreshold(scoreToOptimize, bestThresholdScore);

  if (saveConfusionMatrices)
  {
    BinaryClassificationConfusionMatrix confusionMatrix;
    confusionMatrix.set(numPositives, numNegatives, 0, 0);
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
      
      confusionMatrices.push_back(new BinaryClassificationConfusionMatrix(confusionMatrix));
    }
  }

  predictedScores.clear();
}
