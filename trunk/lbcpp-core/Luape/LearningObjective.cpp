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
#include "NodeBuilder/NodeBuilderTypeSearchSpace.h"
#include "../../lbcpp-ml/Function/SpecialFunctions.h" // for StumpFunction
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

double LearningObjective::computeObjectiveWithEventualStump(ExecutionContext& context, const LuapeInferencePtr& problem, ExpressionPtr& weakNode, const IndexSetPtr& examples)
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
    weakNode = new FunctionExpression(stumpFunction(threshold), weakNode);
    return res;
  }
}

double LearningObjective::findBestThreshold(ExecutionContext& context, const ExpressionPtr& numberNode, const IndexSetPtr& indices, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore, bool verbose)
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
    context.leaveScope(new Pair(bestThresholds.size() ? bestThresholds[0] : 0.0, bestScore));

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
    res.push(supervisions->getValue(*it), getWeight(*it));
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
    double weight = getWeight(example);
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
  double weight = getWeight(index);
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
  doubleVectorClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();
  labels = DoubleVector::getElementsEnumeration(doubleVectorClass);
  numLabels = labels->getNumElements();
  LearningObjective::initialize(problem);
}

/*
 ** InformationGainBinaryLearningObjective
 */
InformationGainBinaryLearningObjective::InformationGainBinaryLearningObjective(bool normalize)
  : normalize(normalize) {}

void InformationGainBinaryLearningObjective::initialize(const LuapeInferencePtr& problem)
{
  static const TypePtr denseVectorClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
  BinaryClassificationLearningObjective::initialize(problem);
  splitWeights = new DenseDoubleVector(3, 0.0); // prediction probabilities
  labelWeights = new DenseDoubleVector(denseVectorClass); // label probabilities
  for (int i = 0; i < 3; ++i)
    labelConditionalProbabilities[i] = new DenseDoubleVector(denseVectorClass); // label probabilities given that the predicted value is negative, positive or missing
}

void InformationGainBinaryLearningObjective::setSupervisions(const VectorPtr& supervisions)
{
  size_t n = supervisions->getNumElements();
  this->supervisions = new GenericVector(booleanType, n);
  for (size_t i = 0; i < n; ++i)
  {
    Variable supervision = supervisions->getElement(i);
    bool label;
    if (lbcpp::convertSupervisionVariableToBoolean(supervision, label))
      this->supervisions->setElement(i, Variable(label, booleanType));
  }
  invalidate();
}

void InformationGainBinaryLearningObjective::update()
{
  splitWeights->multiplyByScalar(0.0);
  labelWeights->multiplyByScalar(0.0);
  for (int i = 0; i < 3; ++i)
    labelConditionalProbabilities[i]->multiplyByScalar(0.0);
  
  sumOfWeights = 0.0;
  for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
  {
    size_t index = it.getIndex();
    double weight = getWeight(index);
    size_t supervision = (int)supervisions->getElement(index).getBoolean() ? 0 : 1;
    unsigned char b = it.getRawBoolean();
    
    splitWeights->incrementValue((size_t)b, weight);
    labelWeights->incrementValue(supervision, weight);
    labelConditionalProbabilities[b]->incrementValue(supervision, weight);
    sumOfWeights += weight;
  }
}

void InformationGainBinaryLearningObjective::flipPrediction(size_t index)
{
  jassert(upToDate);
  size_t supervision = (int)supervisions->getElement(index).getInteger();
  double weight = getWeight(index);
  splitWeights->decrementValue(0, weight); // remove 'false' prediction
  labelConditionalProbabilities[0]->decrementValue(supervision, weight);
  splitWeights->incrementValue(1, weight); // add 'true' prediction
  labelConditionalProbabilities[1]->incrementValue(supervision, weight);
}

double InformationGainBinaryLearningObjective::computeObjective()
{
  ensureIsUpToDate();
  
  double currentEntropy = labelWeights->computeEntropy(sumOfWeights);
  double splitEntropy = splitWeights->computeEntropy(sumOfWeights);
  double expectedNextEntropy = 0.0;
  for (int i = 0; i < 3; ++i)
    expectedNextEntropy += (splitWeights->getValue(i) / sumOfWeights) * labelConditionalProbabilities[i]->computeEntropy(splitWeights->getValue(i));
  double informationGain = currentEntropy - expectedNextEntropy;
  if (normalize)
    return 2.0 * informationGain / (currentEntropy + splitEntropy);
  else
    return informationGain;
}

Variable InformationGainBinaryLearningObjective::computeVote(const IndexSetPtr& indices)
{
  double trueWeight = 0.0;
  double falseWeight = 0.0;
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
  {
    size_t index = *it;
    double weight = getWeight(index);
    if (supervisions->getElement(index).getBoolean())
      trueWeight += weight;
    else
      falseWeight += weight;
  }
  if (trueWeight || falseWeight)
    return Variable(trueWeight / (trueWeight + falseWeight), probabilityType);
  else
    return Variable::missingValue(probabilityType);
}

/*
** InformationGainLearningObjective
*/
InformationGainLearningObjective::InformationGainLearningObjective(bool normalize)
  : normalize(normalize) {}

void InformationGainLearningObjective::initialize(const LuapeInferencePtr& problem)
{
  ClassificationLearningObjective::initialize(problem);
  splitWeights = new DenseDoubleVector(3, 0.0); // prediction probabilities
  labelWeights = new DenseDoubleVector(doubleVectorClass); // label probabilities
  for (int i = 0; i < 3; ++i)
    labelConditionalProbabilities[i] = new DenseDoubleVector(doubleVectorClass); // label probabilities given that the predicted value is negative, positive or missing

  singleVoteVectors.resize(numLabels + 1);
  for (size_t i = 0; i < numLabels; ++i)
  {
    singleVoteVectors[i] = new DenseDoubleVector(doubleVectorClass);
    singleVoteVectors[i]->setValue(i, 1.0);
  }
  singleVoteVectors[numLabels] = new DenseDoubleVector(doubleVectorClass);
}

void InformationGainLearningObjective::setSupervisions(const VectorPtr& supervisions)
{
  size_t n = supervisions->getNumElements();
  this->supervisions = new GenericVector(labels, n);
  for (size_t i = 0; i < n; ++i)
  {
    Variable supervision = supervisions->getElement(i);
    size_t label;
    if (lbcpp::convertSupervisionVariableToEnumValue(supervision, label))
      this->supervisions->setElement(i, Variable(label, labels));
  }
  invalidate();
}

void InformationGainLearningObjective::update()
{
  splitWeights->multiplyByScalar(0.0);
  labelWeights->multiplyByScalar(0.0);
  for (int i = 0; i < 3; ++i)
    labelConditionalProbabilities[i]->multiplyByScalar(0.0);

  sumOfWeights = 0.0;
  for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
  {
    size_t index = it.getIndex();
    double weight = getWeight(index);
    size_t supervision = (int)supervisions->getElement(index).getInteger();
    unsigned char b = it.getRawBoolean();

    splitWeights->incrementValue((size_t)b, weight);
    labelWeights->incrementValue(supervision, weight);
    labelConditionalProbabilities[b]->incrementValue(supervision, weight);
    sumOfWeights += weight;
  }
}

void InformationGainLearningObjective::flipPrediction(size_t index)
{
  jassert(upToDate);
  size_t supervision = (int)supervisions->getElement(index).getInteger();
  double weight = getWeight(index);
  splitWeights->decrementValue(0, weight); // remove 'false' prediction
  labelConditionalProbabilities[0]->decrementValue(supervision, weight);
  splitWeights->incrementValue(1, weight); // add 'true' prediction
  labelConditionalProbabilities[1]->incrementValue(supervision, weight);
}

double InformationGainLearningObjective::computeObjective()
{
  ensureIsUpToDate();

  double currentEntropy = labelWeights->computeEntropy(sumOfWeights);
  double splitEntropy = splitWeights->computeEntropy(sumOfWeights);
  double expectedNextEntropy = 0.0;
  for (int i = 0; i < 3; ++i)
    expectedNextEntropy += (splitWeights->getValue(i) / sumOfWeights) * labelConditionalProbabilities[i]->computeEntropy(splitWeights->getValue(i));
  double informationGain = currentEntropy - expectedNextEntropy;
  if (normalize)
    return 2.0 * informationGain / (currentEntropy + splitEntropy);
  else
    return informationGain;
}

Variable InformationGainLearningObjective::computeVote(const IndexSetPtr& indices)
{
  if (indices->size() == 0)
    return singleVoteVectors[numLabels]; // (a vector of zeros)
  else if (indices->size() == 1)
  {
    // special case when the vote is all concentrated on a single label to spare some memory
    size_t label = (size_t)supervisions->getElement(*indices->begin()).getInteger();
    return singleVoteVectors[label]; // (a vector containing a single 1 on the label)
  }
  else
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(doubleVectorClass);
    double sumOfWeights = 0.0;
    for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    {
      size_t index = *it;
      double weight = getWeight(index);
      res->incrementValue((size_t)supervisions->getElement(index).getInteger(), weight);
      sumOfWeights += weight;
    }
    if (sumOfWeights)
    {
      res->multiplyByScalar(1.0 / sumOfWeights);
      int argmax = res->getIndexOfMaximumValue();
      if (argmax >= 0 && fabs(res->getValue(argmax) - 1.0) < 1e-12)
        return singleVoteVectors[argmax]; // reuse an existing vector to spare memory
    }
    return res;
  }
}
