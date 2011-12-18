/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp               | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeLearner.h>
#include <lbcpp/Data/DoubleVector.h>
#include <algorithm>
using namespace lbcpp;

/*
** LuapeLearner
*/
bool LuapeLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  this->function = function;
  return true;
}

bool LuapeLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  if (isTrainingData)
  {
    trainingData = data;
    trainingCache = function->createSamplesCache(context, data);
    allExamples = new IndexSet(0, data.size());
  }
  else
  {
    validationData = data;
    validationCache = function->createSamplesCache(context, data);
  }
  return true;
}

VectorPtr LuapeLearner::getTrainingPredictions() const
{
  LuapeSampleVectorPtr samples = trainingCache->getSamples(defaultExecutionContext(), function->getRootNode(), allExamples, false);
  jassert(samples->getNumChunks() == 1);
  return samples->getChunkData(0);
}

VectorPtr LuapeLearner::getValidationPredictions() const
{
  if (validationCache)
  {
    LuapeSampleVectorPtr samples = validationCache->getSamples(defaultExecutionContext(), function->getRootNode(), allExamples, false);
    jassert(samples->getNumChunks() == 1);
    return samples->getChunkData(0);
  }
  else
    return VectorPtr();
}

/*
** BoostingLearner
*/
BoostingLearner::BoostingLearner(BoostingWeakLearnerPtr weakLearner)
  : weakLearner(weakLearner) {}

bool BoostingLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  if (!LuapeLearner::initialize(context, function))
    return false;
  jassert(weakLearner);
  return weakLearner->initialize(context, function);
}

LuapeNodePtr BoostingLearner::turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, const IndexSetPtr& examples) const
{
  jassert(weakNode);
  Variable successVote, failureVote, missingVote;
  if (!computeVotes(context, weakNode, examples, successVote, failureVote, missingVote))
    return LuapeNodePtr();

  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  if (weakNode.isInstanceOf<LuapeConstantNode>())
  {
    LuapeConstantNodePtr constantNode = weakNode.staticCast<LuapeConstantNode>();
    Variable constantValue = constantNode->getValue();
    jassert(constantValue.isBoolean());
    if (constantValue.isMissingValue())
      return new LuapeConstantNode(missingVote);
    else if (constantValue.getBoolean())
      return new LuapeConstantNode(successVote);
    else
      return new LuapeConstantNode(failureVote);
  }
  else
    return new LuapeTestNode(weakNode, new LuapeConstantNode(successVote), new LuapeConstantNode(failureVote), new LuapeConstantNode(missingVote));
}

bool BoostingLearner::doLearningIteration(ExecutionContext& context)
{
  LuapeNodePtr contribution;
  double weakObjective;
 
  // do weak learning
  {
    TimedScope _(context, "weak learning");
    contribution = weakLearner->learn(context, refCountedPointerFromThis(this), allExamples, weakObjective);
    if (!contribution)
    {
      context.errorCallback(T("Failed to find a weak learner"));
      return false;
    }
    context.resultCallback(T("edge"), weakObjective);
  }

  // add into node and caches
  {
    TimedScope _(context, "add into node");
    std::vector<LuapeSamplesCachePtr> caches;
    caches.push_back(trainingCache);
    if (validationCache)
      caches.push_back(validationCache);
    function->getRootNode().staticCast<LuapeSequenceNode>()->pushNode(contribution, caches);
  }

  // evaluate
  {
    TimedScope _(context, "evaluate");
    VectorPtr trainingPredictions = getTrainingPredictions();
    context.resultCallback(T("train error"), function->evaluatePredictions(context, trainingPredictions, trainingData));

    VectorPtr validationPredictions = getValidationPredictions();
    if (validationPredictions)
      context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
  }

  // trainingCache->checkCacheIsCorrect(context, function->getRootNode());

  context.resultCallback(T("contribution"), contribution);
  return true;
}

/*
** BoostingWeakObjective
*/
double BoostingWeakObjective::compute(const LuapeSampleVectorPtr& predictions)
{
  setPredictions(predictions);
  return computeObjective();
}

struct SortDoubleValuesOperator
{
  static double transformIntoValidNumber(double input)
    {return input;}

  bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
  {
    double aa = transformIntoValidNumber(a.second);
    double bb = transformIntoValidNumber(b.second);
    return aa == bb ? a.first < b.first : aa < bb;
  }
};

SparseDoubleVectorPtr BoostingWeakObjective::computeSortedDoubleValues(ExecutionContext& context, const LuapeSampleVectorPtr& samples) const
{
  SparseDoubleVectorPtr res = new SparseDoubleVector();
  std::vector< std::pair<size_t, double> >& v = res->getValuesVector();
  size_t n = samples->size();
  v.reserve(n);

  bool isDouble = (samples->getElementsType() == doubleType);
  for (LuapeSampleVector::const_iterator it = samples->begin(); it != samples->end(); ++it)
  {
    double value = isDouble ? it.getRawDouble() : (*it).toDouble();
    if (value != doubleMissingValue)
      v.push_back(std::make_pair(it.getIndex(), value));
  }
  std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  return res;
}

double BoostingWeakObjective::findBestThreshold(ExecutionContext& context, const LuapeSampleVectorPtr& predictions, double& bestScore, bool verbose)
{
  setPredictions(LuapeSampleVector::createConstant(predictions->getIndices(), Variable(false, booleanType)));

  SparseDoubleVectorPtr sortedDoubleValues = computeSortedDoubleValues(context, predictions);
  if (sortedDoubleValues->getNumValues() == 0)
  {
    bestScore = computeObjective();
    return 0.0;
  }

  bestScore = -DBL_MAX;
  double res = 0.0;

  if (verbose)
    context.enterScope("Find best threshold for node");

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
        bestScore = e, res = (threshold + previousThreshold) / 2.0;
      previousThreshold = threshold;
    }
    flipPrediction(index);
  }

  if (verbose)
    context.leaveScope();
  return res;
}

/*
** BoostingWeakLearner
*/
double BoostingWeakLearner::computeWeakObjectiveWithEventualStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr& weakNode, const IndexSetPtr& examples) const
{
  jassert(examples->size());
  if (weakNode->getType() == booleanType)
    return computeWeakObjective(context, structureLearner, weakNode, examples);
  else
  {
    jassert(weakNode->getType()->isConvertibleToDouble());
    double threshold;
    double res = computeWeakObjectiveWithStump(context, structureLearner, weakNode, examples, threshold);
    weakNode = makeStump(structureLearner, weakNode, threshold);
    return res;
  }
}

double BoostingWeakLearner::computeWeakObjective(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& indices) const
{
  LuapeSampleVectorPtr weakPredictions = structureLearner->getTrainingCache()->getSamples(context, weakNode, indices);
  BoostingWeakObjectivePtr edgeCalculator = structureLearner->createWeakObjective();
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  return edgeCalculator->compute(weakPredictions);
}

double BoostingWeakLearner::computeWeakObjectiveWithStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, const IndexSetPtr& indices, double& bestThreshold) const
{
  jassert(indices->size());
  LuapeSampleVectorPtr weakPredictions = structureLearner->getTrainingCache()->getSamples(context, numberNode, indices);
  jassert(weakPredictions->size() == indices->size());
  BoostingWeakObjectivePtr weakObjective = structureLearner->createWeakObjective();
  double bestScore;
  bestThreshold = weakObjective->findBestThreshold(context, weakPredictions, bestScore, false);
  return bestScore;
}

LuapeNodePtr BoostingWeakLearner::makeStump(const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, double threshold) const
  {return structureLearner->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);}

LuapeNodePtr BoostingWeakLearner::makeContribution(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples) const
  {return structureLearner->turnWeakNodeIntoContribution(context, weakNode, examples);}

/*
** FiniteBoostingWeakLearner
*/
LuapeNodePtr FiniteBoostingWeakLearner::learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective) const
{
  const LuapeInferencePtr& function = structureLearner->getFunction();

  std::vector<LuapeNodePtr> weakNodes;
  if (!getCandidateWeakNodes(context, structureLearner, weakNodes))
  {
    context.errorCallback(T("Could not get finite set of candidate weak nodes"));
    return LuapeNodePtr();
  }

  weakObjective = -DBL_MAX;
  LuapeNodePtr bestWeakNode;
  for (size_t i = 0; i < weakNodes.size(); ++i)
  {
    double objective = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNodes[i], examples); // side effect of weakNodes[i]
    if (objective > weakObjective)
      weakObjective = objective, bestWeakNode = weakNodes[i];
  }

  if (!bestWeakNode)
    return LuapeNodePtr();

  return makeContribution(context, structureLearner, bestWeakNode, examples);
}
