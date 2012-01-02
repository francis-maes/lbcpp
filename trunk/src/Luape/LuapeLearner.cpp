/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp               | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeLearner.h>
#include <lbcpp/Luape/LuapeCache.h>
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
    trainingCache->cacheNode(context, function->getRootNode(), VectorPtr(), "Prediction node", false);
  }
  else
  {
    validationData = data;
    validationCache = function->createSamplesCache(context, data);
    validationCache->cacheNode(context, function->getRootNode(), VectorPtr(), "Prediction node", false);
  }
  return true;
}

VectorPtr LuapeLearner::getTrainingPredictions() const
{
  LuapeSampleVectorPtr samples = trainingCache->getSamples(defaultExecutionContext(), function->getRootNode(), trainingCache->getAllIndices());
  jassert(samples->getImplementation() == LuapeSampleVector::cachedVectorImpl);
  return samples->getVector();
}

VectorPtr LuapeLearner::getValidationPredictions() const
{
  if (validationCache)
  {
    LuapeSampleVectorPtr samples = validationCache->getSamples(defaultExecutionContext(), function->getRootNode(), validationCache->getAllIndices());
    jassert(samples->getImplementation() == LuapeSampleVector::cachedVectorImpl);
    return samples->getVector();
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

LuapeNodePtr BoostingLearner::turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, double weakObjective, const IndexSetPtr& indices) const
{
  jassert(weakNode);
  Variable successVote, failureVote, missingVote;
  if (!computeVotes(context, weakNode, indices, successVote, failureVote, missingVote))
    return LuapeNodePtr();

  weakNode->addImportance(weakObjective);

  LuapeNodePtr res;
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  if (weakNode.isInstanceOf<LuapeConstantNode>())
  {
    LuapeConstantNodePtr constantNode = weakNode.staticCast<LuapeConstantNode>();
    Variable constantValue = constantNode->getValue();
    jassert(constantValue.isBoolean());
    if (constantValue.isMissingValue())
      res = new LuapeConstantNode(missingVote);
    else if (constantValue.getBoolean())
      res = new LuapeConstantNode(successVote);
    else
      res = new LuapeConstantNode(failureVote);
  }
  else
    res = new LuapeTestNode(weakNode, new LuapeConstantNode(successVote), new LuapeConstantNode(failureVote), new LuapeConstantNode(missingVote));
  return res;
}

bool BoostingLearner::doLearningIteration(ExecutionContext& context, double& trainingScore, double& validationScore)
{
  LuapeNodePtr contribution;
  double weakObjective;
 
  // do weak learning
  {
    TimedScope _(context, "weak learning", verbose);
    contribution = weakLearner->learn(context, refCountedPointerFromThis(this), trainingCache->getAllIndices(), verbose, weakObjective);
    if (!contribution)
    {
      context.errorCallback(T("Failed to find a weak learner"));
      return false;
    }
    context.resultCallback(T("edge"), weakObjective);
  }

  // add into node and caches
  {
    TimedScope _(context, "add into node", verbose);
    std::vector<LuapeSamplesCachePtr> caches;
    caches.push_back(trainingCache);
    if (validationCache)
      caches.push_back(validationCache);
    function->getRootNode().staticCast<LuapeSequenceNode>()->pushNode(context, contribution, caches);
  }

  // evaluate
  {
    TimedScope _(context, "evaluate", verbose);
    VectorPtr trainingPredictions = getTrainingPredictions();
    trainingScore = function->evaluatePredictions(context, trainingPredictions, trainingData);
    context.resultCallback(T("train error"), trainingScore);

    VectorPtr validationPredictions = getValidationPredictions();
    if (validationPredictions)
    {
      validationScore = function->evaluatePredictions(context, validationPredictions, validationData);
      context.resultCallback(T("validation error"), validationScore);
    }
    else
      validationScore = 0.0;
  }

  // trainingCache->checkCacheIsCorrect(context, function->getRootNode());
  if (verbose)
    context.resultCallback(T("contribution"), verbose ? Variable(contribution) : Variable(contribution->toShortString()));
  return true;
}
