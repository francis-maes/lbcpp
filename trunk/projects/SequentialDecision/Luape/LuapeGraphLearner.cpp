/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphLearner.cpp          | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeGraphLearner.h"
using namespace lbcpp;

/*
** LuapeGraphLearner
*/
bool LuapeGraphLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
{
  this->problem = problem;
  this->function = function;
  this->graph = function->getGraph();
  return problem->getObjective()->initialize(context, problem, function);
}

bool LuapeGraphLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  graph->clearSamples(isTrainingData, !isTrainingData);
  problem->getObjective()->setExamples(isTrainingData, data);
  return true;
}

/*
** LuapeGradientBoostingLearner
*/
LuapeGradientBoostingLearner::LuapeGradientBoostingLearner(double learningRate, size_t maxDepth)
  : learningRate(learningRate), maxDepth(maxDepth)
{
}

bool LuapeGradientBoostingLearner::doLearningIteration(ExecutionContext& context)
{
  LuapeObjectivePtr objective = problem->getObjective();

  // 1- compute graph outputs and compute loss derivatives
  context.enterScope(T("Computing predictions"));
  DenseDoubleVectorPtr predictions = function->computeSamplePredictions(context, true);
  context.leaveScope();
  context.enterScope(T("Computing pseudo-residuals"));
  double lossValue;
  DenseDoubleVectorPtr pseudoResiduals;
  objective->computeLoss(predictions, &lossValue, &pseudoResiduals);
  context.leaveScope();
  context.resultCallback(T("loss"), lossValue);

  context.resultCallback(T("predictions"), predictions);
  context.resultCallback(T("pseudoResiduals"), pseudoResiduals);

  // 2- find best weak learner
  LuapeNodePtr weakLearner = doWeakLearning(context, predictions, pseudoResiduals);
  if (!weakLearner)
    return false;

  // 3- add weak learner to graph
  if (!addWeakLearnerToGraph(context, predictions, pseudoResiduals, weakLearner))
    return false;

  // ok
  return true;
}

bool LuapeGradientBoostingLearner::addWeakLearnerToGraph(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, const DenseDoubleVectorPtr& pseudoResiduals, LuapeNodePtr completion)
{
  // 1- find optimal weight
  context.informationCallback(T("Weak learner: ") + completion->toShortString());
  LuapeNodeCachePtr weakLearnerCache = completion->getCache();
  BooleanVectorPtr weakPredictions = weakLearnerCache->getSamples(true).staticCast<BooleanVector>();
  double optimalWeight = problem->getObjective()->optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
  context.informationCallback(T("Optimal weight: ") + String(optimalWeight));
  
  // 2- add weak learner and associated weight to graph 
  graph->pushMissingNodes(context, completion);
  if (completion->getType() == booleanType)
  {
    // booleans are yielded directory
    graph->pushNode(context, new LuapeYieldNode(completion));
  }
  else
  {
    jassert(completion->getType()->isConvertibleToDouble());
    
    jassert(false); // todo: create stump
  }
  function->getVotes()->append(optimalWeight * learningRate);
  context.informationCallback(T("Graph: ") + graph->toShortString());
  return true;
}
