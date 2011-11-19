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

double LuapeGradientBoostingLearner::computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const
{
  static const double p = 0.5;
  
  RandomGeneratorPtr random = context.getRandomGenerator();

  completion->updateCache(context, true);

  ContainerPtr samples = completion->getCache()->getTrainingSamples();
  DenseDoubleVectorPtr scalars = samples.dynamicCast<DenseDoubleVector>();
  if (scalars)
  {
    jassert(false); // FIXME: evaluate quality of decision stump
    return 0.0;
  }
  else
  {
    BooleanVectorPtr booleans = samples.dynamicCast<BooleanVector>();
    jassert(booleans);
    size_t n = booleans->getNumElements();
    jassert(n == pseudoResiduals->getNumValues());

    ScalarVariableMeanAndVariance trainPositive;
    ScalarVariableMeanAndVariance validationPositive;
    ScalarVariableMeanAndVariance trainNegative;
    ScalarVariableMeanAndVariance validationNegative;

    for (size_t i = 0; i < n; ++i)
    {
      bool isPositive = booleans->get(i);

      double value = pseudoResiduals->getValue(i);
      double weight = 1.0; // fabs(value)

      //if (random->sampleBool(p))
        (isPositive ? trainPositive : trainNegative).push(value, weight);
      //else
        (isPositive ? validationPositive : validationNegative).push(value, weight);
    }
    
    double meanSquareError = 0.0;
    if (validationPositive.getCount())
      meanSquareError += validationPositive.getCount() * (trainPositive.getSquaresMean() + validationPositive.getSquaresMean() 
                                                            - 2 * trainPositive.getMean() * validationPositive.getMean());
    if (validationNegative.getCount())
      meanSquareError += validationNegative.getCount() * validationNegative.getSquaresMean(); // when negative, we always predict 0
//      meanSquareError += validationNegative.getCount() * (trainNegative.getSquaresMean() + validationNegative.getSquaresMean() 
//                                                            - 2 * trainNegative.getMean() * validationNegative.getMean());

    if (validationPositive.getCount() || validationNegative.getCount())
      meanSquareError /= validationPositive.getCount() + validationNegative.getCount();
    
    return 1.0 - sqrt(meanSquareError) / 2.0; // ... bring into [0,1]
  }
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
  objective->computeLoss(predictions, &lossValue, &pseudoResiduals);
  context.leaveScope();
  context.resultCallback(T("loss"), lossValue);

  context.resultCallback(T("predictions"), predictions);
  context.resultCallback(T("pseudoResiduals"), pseudoResiduals);

  // 2- find best weak learner
  LuapeNodePtr weakLearner = doWeakLearning(context, predictions);
  if (!weakLearner)
    return false;

  // 3- add weak learner to graph
  if (!addWeakLearnerToGraph(context, predictions, weakLearner))
    return false;

  // ok
  return true;
}

bool LuapeGradientBoostingLearner::addWeakLearnerToGraph(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, LuapeNodePtr completion)
{
  // 1- compute optimal weight
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
