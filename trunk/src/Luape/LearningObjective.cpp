/*-----------------------------------------.---------------------------------.
| Filename: LearningObjective.cpp          | Learning Objective              |
| Author  : Francis Maes                   |   base classes                  |
| Started : 22/12/2011 14:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LearningObjective.h>
#include <lbcpp/Luape/LuapeCache.h>
#include <lbcpp/Learning/Numerical.h> // for lbcpp::convertSupervisionVariableToEnumValue
#include "NodeBuilder/NodeBuilderTypeSearchSpace.h"
#include "Function/SpecialLuapeFunctions.h" // for StumpLuapeFunction
using namespace lbcpp;

/*
** LearningObjective
*/
void LearningObjective::ensureIsUpToDate()
{
  if (!upToDate)
  {
    update();
    upToDate = true;
  }
}

double LearningObjective::compute(const LuapeSampleVectorPtr& predictions)
{
  setPredictions(predictions);
  return computeObjective();
}

double LearningObjective::computeObjectiveWithEventualStump(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeNodePtr& weakNode, const IndexSetPtr& examples)
{
  jassert(examples->size());
  if (weakNode->getType() == booleanType)
  {
    LuapeSampleVectorPtr weakPredictions = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    return compute(weakPredictions);
  }
  else
  {
    jassert(weakNode->getType()->isConvertibleToDouble());
    double res;
    SparseDoubleVectorPtr sortedDoubleValues = problem->getTrainingCache()->getSortedDoubleValues(context, weakNode, examples);
    double threshold = findBestThreshold(context, weakNode, examples, sortedDoubleValues, res, false);
    weakNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), weakNode);
    return res;
  }
}

double LearningObjective::findBestThreshold(ExecutionContext& context, const LuapeNodePtr& numberNode, const IndexSetPtr& indices, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore, bool verbose)
{
  setPredictions(LuapeSampleVector::createConstant(indices, Variable(false, booleanType)));
  ensureIsUpToDate();

  if (sortedDoubleValues->getNumValues() == 0)
  {
    bestScore = computeObjective();
    return 0.0;
  }

  bestScore = -DBL_MAX;
  std::vector<double> bestThresholds;

  if (verbose)
    context.enterScope("Find best threshold for node " + numberNode->toShortString());

  size_t n = sortedDoubleValues->getNumValues();
  double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
  for (int i = (int)n - 1; i >= 0; --i)
  {
    size_t index = sortedDoubleValues->getValue(i).first;
    double threshold = sortedDoubleValues->getValue(i).second;

    jassert(threshold <= previousThreshold);
    if (threshold < previousThreshold)
    {
      double e = computeObjective();

      if (verbose)
      {
        context.enterScope("Iteration " + String((int)i));
        context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
        context.resultCallback("edge", e);
        context.leaveScope();
      }

      if (e >= bestScore)
      {
        if (e > bestScore)
        {
          bestThresholds.clear();
          bestScore = e;
        }
        bestThresholds.push_back((threshold + previousThreshold) / 2.0);
      }
      previousThreshold = threshold;
    }
    flipPrediction(index);
  }

  if (verbose)
    context.leaveScope();

  return bestThresholds.size() ? bestThresholds[bestThresholds.size() / 2] : 0; // median value
}

/*
** RegressionLearningObjective
*/
void RegressionLearningObjective::setSupervisions(const VectorPtr& supervisions)
{
  jassert(supervisions->getElementsType() == doubleType);
  this->supervisions = supervisions.staticCast<DenseDoubleVector>();
  invalidate();
}

Variable RegressionLearningObjective::computeVote(const IndexSetPtr& indices)
{
  ScalarVariableMean res;
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    res.push(supervisions->getValue(*it), getWeight(*it));
  return res.getMean();
}

void RegressionLearningObjective::update()
{
  positives.clear();
  negatives.clear();
  missings.clear();

  for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
  {
    double value = supervisions->getValue(it.getIndex());
    double weight = getWeight(it.getIndex());
    switch (it.getRawBoolean())
    {
    case 0: negatives.push(value, weight); break;
    case 1: positives.push(value, weight); break;
    case 2: missings.push(value, weight); break;
    default: jassert(false);
    }
  }
}

void RegressionLearningObjective::flipPrediction(size_t index)
{
  jassert(upToDate);

  double value = supervisions->getValue(index);
  double weight = getWeight(index);
  negatives.push(value, -weight);
  positives.push(value, weight);
}

double RegressionLearningObjective::computeObjective()
{
  double res = 0.0;
  if (positives.getCount())
    res += positives.getCount() * positives.getVariance();
  if (negatives.getCount())
    res += negatives.getCount() * negatives.getVariance();
  if (missings.getCount())
    res += missings.getCount() * missings.getVariance();
  jassert(predictions->size() == (size_t)(positives.getCount() + negatives.getCount() + missings.getCount()));
  if (predictions->size())
    res /= (double)predictions->size();
  return -res;
}

/*
** BinaryClassificationLearningObjective
*/
BinaryClassificationLearningObjective::BinaryClassificationLearningObjective()
  : correctWeight(0.0), errorWeight(0.0), missingWeight(0.0)
{
}

void BinaryClassificationLearningObjective::setSupervisions(const VectorPtr& supervisions)
{
  jassert(supervisions->getElementsType() == probabilityType);
  this->supervisions = supervisions.staticCast<DenseDoubleVector>();
  invalidate();
}

Variable BinaryClassificationLearningObjective::computeVote(const IndexSetPtr& indices)
{
  ScalarVariableMean res;
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    res.push(supervisions->getValue(*it), weights->getValue(*it));
  return Variable(res.getMean(), probabilityType);
}

void BinaryClassificationLearningObjective::update()
{
  correctWeight = 0.0;
  errorWeight = 0.0;
  missingWeight = 0.0;

  for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
  {
    size_t example = it.getIndex();
    bool sup = (supervisions->getValue(example) > 0.5);
    double weight = weights->getValue(example);
    unsigned char pred = it.getRawBoolean();
    if (pred == 2)
      missingWeight += weight;
    else if ((pred == 0 && !sup) || (pred == 1 && sup))
      correctWeight += weight;
    else
      errorWeight += weight;
  }
}

void BinaryClassificationLearningObjective::flipPrediction(size_t index)
{
  jassert(upToDate);
  bool sup = supervisions->getValue(index) > 0.5;
  double weight = weights->getValue(index);
  if (sup)
  {
    correctWeight += weight;
    errorWeight -= weight;
  }
  else
  {
    errorWeight += weight;
    correctWeight -= weight;
  }
}

double BinaryClassificationLearningObjective::computeObjective()
{
  ensureIsUpToDate();
  double totalWeight = (missingWeight + correctWeight + errorWeight);
  jassert(totalWeight);
  return juce::jmax(correctWeight / totalWeight, errorWeight / totalWeight);
}

/*
** ClassificationLearningObjective
*/
void ClassificationLearningObjective::initialize(const LuapeInferencePtr& problem)
{
  LearningObjective::initialize(problem);
  doubleVectorClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();
  labels = DoubleVector::getElementsEnumeration(doubleVectorClass);
  numLabels = labels->getNumElements();
  for (size_t i = 0; i < 3; ++i)
    for (size_t j = 0; j < 2; ++j)
      mu[i][j] = new DenseDoubleVector(doubleVectorClass, numLabels);
}

void ClassificationLearningObjective::setSupervisions(const VectorPtr& sup)
{
  EnumerationPtr labels = LuapeClassifier::getLabelsFromSupervision(sup->getElementsType());
  size_t n = sup->getNumElements();
  size_t m = labels->getNumElements();
  supervisions = new DenseDoubleVector(n * m, 0.0);
  size_t index = 0;
  for (size_t i = 0; i < n; ++i)
  {
    Variable supervision = sup->getElement(i);
    size_t label;
    if (!lbcpp::convertSupervisionVariableToEnumValue(supervision, label))
      jassertfalse;
    for (size_t j = 0; j < m; ++j, ++index)
      supervisions->setValue(index, j == label ? 1.0 : -1.0);
  }
}

void ClassificationLearningObjective::update()
{
  if (!weights)
  {
    jassert(supervisions);
    weights = makeDefaultWeights(supervisions);
  }
  jassert(supervisions->getNumValues() == weights->getNumValues());
  for (size_t i = 0; i < 3; ++i)
    for (size_t j = 0; j < 2; ++j)
      mu[i][j]->multiplyByScalar(0.0);

  for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
  {
    size_t example = it.getIndex();
    unsigned char prediction = it.getRawBoolean();
    double* weightsPtr = weights->getValuePointer(numLabels * example);
    double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
    for (size_t j = 0; j < numLabels; ++j)
    {
      double supervision = *supervisionsPtr++;
      mu[prediction][supervision > 0 ? 1 : 0]->incrementValue(j, *weightsPtr++);
    }
  }
}

void ClassificationLearningObjective::flipPrediction(size_t index)
{
  jassert(upToDate);
  double* weightsPtr = weights->getValuePointer(index * numLabels);
  double* muNegNegPtr = mu[0][0]->getValuePointer(0);
  double* muNegPosPtr = mu[0][1]->getValuePointer(0);
  double* muPosNegPtr = mu[1][0]->getValuePointer(0);
  double* muPosPosPtr = mu[1][1]->getValuePointer(0);
  double* supervisionsPtr = supervisions->getValuePointer(index * numLabels);
  for (size_t i = 0; i < numLabels; ++i)
  {
    double weight = *weightsPtr++;
    
    double& muNegNeg = *muNegNegPtr++;
    double& muNegPos = *muNegPosPtr++;
    double& muPosNeg = *muPosNegPtr++;
    double& muPosPos = *muPosPosPtr++;
    double supervision = *supervisionsPtr++;
    
    if (supervision < 0)
      muNegNeg -= weight, muPosNeg += weight;
    else
      muNegPos -= weight, muPosPos += weight;
  }
}

Variable ClassificationLearningObjective::computeVote(const IndexSetPtr& indices)
{
  std::vector<ScalarVariableMean> stats(numLabels);
  
  DenseDoubleVectorPtr res = new DenseDoubleVector(labels, probabilityType);
  double sum = 0.0;
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
  {
    size_t example = *it;
    double* supervisionsPtr = supervisions->getValuePointer(numLabels * example);
    double* weightsPtr = weights->getValuePointer(numLabels * example);
    for (size_t i = 0; i < numLabels; ++i)
    {
      double value = *weightsPtr++ * (*supervisionsPtr++ + 1.0) / 2.0; // transform signed supervision into probability
      res->incrementValue(i, value);
      sum += value;
    }
  }
  if (sum)
    res->multiplyByScalar(1.0 / sum); // normalize probability distribution
  return Variable(res, denseDoubleVectorClass(labels, probabilityType));
}

double ClassificationLearningObjective::computeObjective()
{
  ensureIsUpToDate();
  size_t n = labels->getNumElements();
  double edge = 0.0;

  const double* muNegNegPtr = mu[0][0]->getValuePointer(0);
  const double* muPosNegPtr = mu[1][0]->getValuePointer(0);
  const double* muNegPosPtr = mu[0][1]->getValuePointer(0);
  const double* muPosPosPtr = mu[1][1]->getValuePointer(0);
  
  for (size_t j = 0; j < n; ++j)
  {
    double muPositive = (*muNegNegPtr++) + (*muPosPosPtr++);
    double muNegative = (*muPosNegPtr++) + (*muNegPosPtr++);
    double vote = (muPositive > muNegative + 1e-9 ? 1.0 : -1.0);
    edge += vote * (muPositive - muNegative);
  }
  return edge;
}

DenseDoubleVectorPtr ClassificationLearningObjective::makeDefaultWeights(const DenseDoubleVectorPtr& supervisions) const
{
  size_t n = supervisions->getNumValues();
  size_t numExamples = n / numLabels;
  double positiveWeight =  1.0 / (2 * numExamples);
  double negativeWeight = 1.0 / (2 * numExamples * (numLabels - 1));
  DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
  for (size_t i = 0; i < n; ++i)
    res->setValue(i, supervisions->getValue(i) > 0.0 ? positiveWeight : negativeWeight);
  return res;
}
