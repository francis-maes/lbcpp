/*-----------------------------------------.---------------------------------.
| Filename: BinaryConfusionMatrix.cpp      | Binary Confusion Matrix         |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/BinaryConfusionMatrix.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/DefaultClass.h>
#include <lbcpp/Core/Integer.h>
using namespace lbcpp;

BinaryConfusionMatrix::BinaryConfusionMatrix(const BinaryConfusionMatrix& other)
  : truePositive(other.truePositive),
    falsePositive(other.falsePositive),
    falseNegative(other.falseNegative),
    trueNegative(other.trueNegative),
    totalCount(other.totalCount)
{
}

BinaryConfusionMatrix::BinaryConfusionMatrix()
  : truePositive(0),
    falsePositive(0),
    falseNegative(0),
    trueNegative(0),
    totalCount(0)
{
}

inline string toFixedLengthString(const string& str, int size)
{
  jassert(str.length() <= size);
  string res = str;
  while (res.length() < size)
    if (res.length() % 2 == 0)
      res = T(" ") + res;
    else
      res += T(" ");
  return res;
}

string BinaryConfusionMatrix::toString() const
{
  return toFixedLengthString(T("Actual value: "), 20) + toFixedLengthString(T("positive"), 15) + toFixedLengthString(T("negative"), 15) + T("\n") +
    toFixedLengthString(T("Predicted as pos.: "), 20) + toFixedLengthString(string((int)truePositive), 15) + toFixedLengthString(string((int)falsePositive), 15) + T("\n") +
    toFixedLengthString(T("Predicted as neg.: "), 20) + toFixedLengthString(string((int)falseNegative), 15) + toFixedLengthString(string((int)trueNegative), 15) + T("\n")
    + T("ACC = ") + string(computeAccuracy() * 100.0, 2)
    + T("% P = ") + string(computePrecision() * 100.0, 2)
    + T("% R = ") + string(computeRecall() * 100.0, 2)
    + T("% F1 = ") + string(computeF1Score() * 100.0, 2)
    + T("% MCC = ") + string(computeMatthewsCorrelation(), 4) + T("\n");
}

bool BinaryConfusionMatrix::convertToBoolean(ExecutionContext& context, const ObjectPtr& object, bool& res)
{
  if (!object || !object->getClass()->isConvertibleToBoolean())
    return false;
  res = object->toBoolean();
  return true;
}

void BinaryConfusionMatrix::clear()
{
  truePositive = falsePositive = falseNegative = trueNegative = totalCount = 0;
}

void BinaryConfusionMatrix::set(size_t truePositive, size_t falsePositive, size_t falseNegative, size_t trueNegative)
{
  this->truePositive = truePositive;
  this->falsePositive = falsePositive;
  this->falseNegative = falseNegative;
  this->trueNegative = trueNegative;
  totalCount = truePositive + falsePositive + falseNegative + trueNegative;
}

void BinaryConfusionMatrix::addPrediction(bool predicted, bool correct, size_t count)
{
  if (predicted)
    correct ? (truePositive += count) : (falsePositive += count);
  else
    correct ? (falseNegative += count) : (trueNegative += count);
  totalCount += count;
}

void BinaryConfusionMatrix::addPredictionIfExists(ExecutionContext& context, const ObjectPtr& predicted, const ObjectPtr& correct, size_t count)
{
  bool p, c;
  if (convertToBoolean(context, predicted, p) && convertToBoolean(context, correct, c))
    addPrediction(p, c, count);
}

void BinaryConfusionMatrix::removePrediction(bool predicted, bool correct, size_t count)
{
  jassert(totalCount);
  if (predicted)
    correct ? (truePositive -= count) : (falsePositive -= count);
  else
    correct ? (falseNegative -= count) : (trueNegative -= count);
  totalCount -= count;
}

size_t BinaryConfusionMatrix::getCount(bool predicted, bool correct) const
{
  return predicted
    ? (correct ? truePositive : falsePositive)
    : (correct ? falseNegative : trueNegative);
}

double BinaryConfusionMatrix::computeMatthewsCorrelation() const
{
  size_t positiveCount = truePositive + falseNegative;
  size_t negativeCount = falsePositive + trueNegative;
  
  size_t predictedPositiveCount = truePositive + falsePositive;
  size_t predictedNegativeCount = falseNegative + trueNegative;
  
  double mccNo = (double)(truePositive * trueNegative) - (double)(falsePositive * falseNegative);
  double mccDeno = (double)positiveCount * (double)negativeCount * (double)predictedPositiveCount * (double)predictedNegativeCount;
  return mccDeno ? (mccNo / sqrt(mccDeno)) : mccNo;
}

double BinaryConfusionMatrix::computeAccuracy() const
  {return totalCount ? (truePositive + trueNegative) / (double)totalCount : 0.0;}

void BinaryConfusionMatrix::computePrecisionRecallAndF1(double& precision, double& recall, double& f1score) const
{
  precision = (truePositive || falsePositive) ? truePositive / (double)(truePositive + falsePositive) : 0.0;
  recall = (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;
  f1score = precision + recall > 0.0 ? (2.0 * precision * recall / (precision + recall)) : 0.0;
}

double BinaryConfusionMatrix::computeF1Score() const
{
  double denom = truePositive + (falseNegative + falsePositive) / 2.0;
  return denom ? truePositive / (double)denom : 1.0;
}

double BinaryConfusionMatrix::computePrecision() const
  {return (truePositive || falsePositive) ? truePositive / (double)(truePositive + falsePositive) : 0.0;}

double BinaryConfusionMatrix::computeRecall() const
  {return (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;}

double BinaryConfusionMatrix::computeSensitivity() const
  {return (truePositive || falseNegative) ? truePositive / (double)(truePositive + falseNegative) : 0.0;}

double BinaryConfusionMatrix::computeSpecificity() const
  {return (trueNegative || falsePositive) ? trueNegative / (double)(trueNegative + falsePositive) : 0.0;}

double BinaryConfusionMatrix::computeSensitivityAndSpecificity() const
{
  const double sensitivity = computeSensitivity();
  const double specificity = computeSpecificity();
  const double sum = sensitivity + specificity;
  if (sum < 10e-6)
    return 0.0;
  return 2 * (sensitivity * specificity) / sum;
  //return (computeSensitivity() + computeSpecificity() / 2;
}

void BinaryConfusionMatrix::saveToXml(XmlExporter& exporter) const
{
  string res = string((int)truePositive) + T(" ")
             + string((int)falsePositive) + T(" ")
             + string((int)falseNegative) + T(" ")
             + string((int)trueNegative);
  exporter.addTextElement(res);
}

bool BinaryConfusionMatrix::loadFromXml(XmlImporter& importer)
{
  StringArray tokens;
  tokens.addTokens(importer.getAllSubText(), true);
  if (tokens.size() != 4)
    return false;

  ObjectPtr v = Object::createFromString(importer.getContext(), positiveIntegerClass, tokens[0]);
  if (!v)
    return false;
  truePositive = PositiveInteger::get(v);

  v = Object::createFromString(importer.getContext(), positiveIntegerClass, tokens[1]);
  if (!v.exists())
    return false;
  falsePositive = PositiveInteger::get(v);

  v = Object::createFromString(importer.getContext(), positiveIntegerClass, tokens[2]);
  if (!v.exists())
    return false;
  falseNegative = PositiveInteger::get(v);

  v = Object::createFromString(importer.getContext(), positiveIntegerClass, tokens[3]);
  if (!v.exists())
    return false;
  trueNegative = PositiveInteger::get(v);

  totalCount = truePositive + falsePositive + falseNegative + trueNegative;
  return true;
}

bool BinaryConfusionMatrix::operator ==(const BinaryConfusionMatrix& other) const
{
  return truePositive == other.truePositive && falsePositive == other.falsePositive && 
    falseNegative == other.falseNegative && trueNegative == other.trueNegative && totalCount == other.totalCount;
}
