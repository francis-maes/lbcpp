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
bool LuapeLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
{
  this->problem = problem;
  this->function = function;
  if (!function->getVotes())
    function->setVotes(function->createVoteVector(0));
  if (!function->getGraph())
    function->setGraph(problem->createInitialGraph(context));
  this->graph = function->getGraph();
  return !problem->getObjective() || problem->getObjective()->initialize(context, problem, function);
}

bool LuapeLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  graph->clearSamples(isTrainingData, !isTrainingData);
  if (problem->getObjective())
    problem->getObjective()->setExamples(isTrainingData, data);
  return true;
}

/*
** BoostingLearner
*/
BoostingLearner::BoostingLearner(BoostingWeakLearnerPtr weakLearner)
  : weakLearner(weakLearner) {}

bool BoostingLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
{
  if (!LuapeLearner::initialize(context, problem, function))
    return false;
  jassert(weakLearner);
  return weakLearner->initialize(context, problem, function);
}

LuapeNodePtr BoostingLearner::createDecisionStump(ExecutionContext& context, const LuapeNodePtr& numberNode) const
{
  double threshold = computeBestStumpThreshold(context, numberNode);
  return graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);
}

LuapeNodePtr BoostingLearner::doWeakLearning(ExecutionContext& context) const
{
  LuapeNodePtr weakNode = weakLearner->learn(context, refCountedPointerFromThis(this));
  if (!weakNode)
    context.errorCallback(T("Failed to find a weak learner"));
  if (weakNode->getType() != booleanType)
    weakNode = createDecisionStump(context, weakNode); // transforms doubles into decision stumps
  return weakNode;
}

LuapeNodePtr BoostingLearner::doWeakLearningAndAddToGraph(ExecutionContext& context, BooleanVectorPtr& weakPredictions)
{
  LuapeNodePtr weakNode;

  // do weak learning
  {
    TimedScope _(context, "weak learning");
    weakNode = doWeakLearning(context);
    if (!weakNode)
      return LuapeNodePtr();
  }
 
  {
    TimedScope _(context, "add to graph");

    // add missing nodes to graph
    jassert(weakNode->getType() == booleanType);
    graph->pushMissingNodes(context, new LuapeYieldNode(weakNode));

    // update the weak learner
    weakLearner->update(context, refCountedPointerFromThis(this), weakNode);

    // retrieve weak predictions
    weakNode->updateCache(context, true);
    jassert(weakNode->getCache()->getNumTrainingSamples() > 0);
    weakPredictions = weakNode->getCache()->getSamples(true).staticCast<BooleanVector>();
  }
  return weakNode;
}
