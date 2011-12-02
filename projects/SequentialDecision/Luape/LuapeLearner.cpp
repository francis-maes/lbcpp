/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp          | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeLearner.h"
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
  // we store a local copy to perform (fast) evaluation at each iteration
  if (isTrainingData)
  {
    trainingData = data;
    trainingSamples = function->createSamplesCache(context, data);
  }
  else
  {
    validationData = data;
    validationSamples = function->createSamplesCache(context, data);
  }
  return true;
}

VectorPtr LuapeLearner::getTrainingPredictions() const
{
  return trainingSamples->compute(defaultExecutionContext(), function->getRootNode(), false);
}

VectorPtr LuapeLearner::getValidationPredictions() const
{
  if (validationSamples)
    return validationSamples->compute(defaultExecutionContext(), function->getRootNode(), false);
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

bool BoostingLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  if (!LuapeLearner::setExamples(context, isTrainingData, data))
    return false;
  if (isTrainingData)
  {
    allExamples.resize(data.size());
    for (size_t i = 0; i < allExamples.size(); ++i)
      allExamples[i] = i;
  }
  return true;
}

LuapeNodePtr BoostingLearner::turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const
{
  jassert(weakNode);
  Variable successVote, failureVote;
  if (!computeVotes(context, weakNode, examples, successVote, failureVote))
    return LuapeNodePtr();
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  return new LuapeTestNode(weakNode, new LuapeConstantNode(successVote), new LuapeConstantNode(failureVote));
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
    caches.push_back(trainingSamples);
    if (validationSamples)
      caches.push_back(validationSamples);
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

  context.resultCallback(T("contribution"), contribution);
  return true;
}

/*
** BoostingWeakObjective
*/
double BoostingWeakObjective::compute(const VectorPtr& predictions)
{
  setPredictions(predictions);
  return computeObjective();
}

double BoostingWeakObjective::findBestThreshold(ExecutionContext& context, size_t numSamples, const SparseDoubleVectorPtr& sortedDoubleValues, double& edge, bool verbose)
{
  setPredictions(new DenseDoubleVector(numSamples, 1.0));

  edge = -DBL_MAX;
  double res = 0.0;

  if (verbose)
    context.enterScope("Find best threshold for node");

  jassert(sortedDoubleValues->getNumValues());
  double previousThreshold = sortedDoubleValues->getValue(0).second;
  for (size_t i = 0; i < sortedDoubleValues->getNumValues(); ++i)
  {
    double threshold = sortedDoubleValues->getValue(i).second;

    jassert(threshold >= previousThreshold);
    if (threshold > previousThreshold)
    {
      double e = computeObjective();

      if (verbose)
      {
        context.enterScope("Iteration " + String((int)i));
        context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
        context.resultCallback("edge", e);
        context.leaveScope();
      }

      if (e >= edge)
        edge = e, res = (threshold + previousThreshold) / 2.0;
      previousThreshold = threshold;
    }
    flipPrediction(sortedDoubleValues->getValue(i).first);
  }

  if (verbose)
    context.leaveScope();
  return res;
}

/*
** BoostingWeakLearner
*/
double BoostingWeakLearner::computeWeakObjectiveWithEventualStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const
{
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

double BoostingWeakLearner::computeWeakObjective(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const
{
  BoostingWeakObjectivePtr edgeCalculator = structureLearner->createWeakObjective(examples);
  VectorPtr weakPredictions = structureLearner->getTrainingSamples()->compute(context, weakNode);
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  return edgeCalculator->compute(weakPredictions);
}

double BoostingWeakLearner::computeWeakObjectiveWithStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, const std::vector<size_t>& examples, double& bestThreshold) const
{
  LuapeSamplesCachePtr trainingSamples = structureLearner->getTrainingSamples();
  SparseDoubleVectorPtr sortedDoubleValues = trainingSamples->getSortedDoubleValues(context, numberNode, examples);
  BoostingWeakObjectivePtr edgeCalculator = structureLearner->createWeakObjective(examples);
  double edge;
  bestThreshold = edgeCalculator->findBestThreshold(context, trainingSamples->getNumSamples(), sortedDoubleValues, edge, false);
  return edge;
}

LuapeNodePtr BoostingWeakLearner::makeStump(const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, double threshold) const
  {return structureLearner->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);}

LuapeNodePtr BoostingWeakLearner::makeContribution(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const
  {return structureLearner->turnWeakNodeIntoContribution(context, weakNode, examples);}
