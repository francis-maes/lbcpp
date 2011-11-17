/*-----------------------------------------.---------------------------------.
| Filename: LuapeGradientBoosting.cpp      | Luape Gradient Boosting         |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeGradientBoosting.h"
using namespace lbcpp;

/*
** LuapeGraphBuilderBanditPool
*/
LuapeGraphBuilderBanditPool::LuapeGraphBuilderBanditPool(size_t maxSize, size_t maxDepth)
  : maxSize(maxSize), maxDepth(maxDepth)
{
}

void LuapeGraphBuilderBanditPool::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph)
{
  LuapeRPNGraphBuilderStatePtr builder = new LuapeRPNGraphBuilderState(problem, graph, maxDepth);
  createNewArms(context, builder);
  createBanditsQueue();
}

void LuapeGraphBuilderBanditPool::executeArm(ExecutionContext& context, size_t armIndex,  const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
{
  // update arms
  destroyArm(context, armIndex);

  LuapeRPNGraphBuilderStatePtr builder = new LuapeRPNGraphBuilderState(problem, graph, maxDepth);
  double reward;
  builder->performTransition(context, newNode, reward); // push last created node
  createNewArms(context, builder);
  createBanditsQueue();

  context.informationCallback(String((int)arms.size()) + T(" arms"));
}

// train the weak learner on 50% of data and evaluate on the other 50% of data
double LuapeGraphBuilderBanditPool::sampleReward(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals, size_t armIndex) const
{
  static const double p = 0.5;
  
  RandomGeneratorPtr random = context.getRandomGenerator();

  LuapeNodePtr node = arms[armIndex].node;
  node->updateCache(context, true);

  ContainerPtr samples = node->getCache()->getTrainingSamples();
  DenseDoubleVectorPtr scalars = samples.dynamicCast<DenseDoubleVector>();
  if (scalars)
  {
    jassert(false); // not implemented yet: todo: random split or heuristic split
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

      //if (random->sampleBool(p))
        (isPositive ? trainPositive : trainNegative).push(value, fabs(value));
      //else
        (isPositive ? validationPositive : validationNegative).push(value, fabs(value));
    }
    
    double meanSquareError = 0.0;
    if (validationPositive.getCount())
      meanSquareError += validationPositive.getCount() * (trainPositive.getSquaresMean() + validationPositive.getSquaresMean() 
                                                            - 2 * trainPositive.getMean() * validationPositive.getMean());
    if (validationNegative.getCount())
      meanSquareError += validationNegative.getCount() * validationNegative.getSquaresMean(); // when negative, we always predict 0

    if (validationPositive.getCount() || validationNegative.getCount())
      meanSquareError /= validationPositive.getCount() + validationNegative.getCount();
    
    return 1.0 - meanSquareError;
  }
}

void LuapeGraphBuilderBanditPool::playArmWithHighestIndex(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals)
{
  size_t armIndex = banditsQueue.top().first;
  banditsQueue.pop();
  
  Arm& arm = arms[armIndex];
  ++arm.playedCount;
  arm.rewardSum += sampleReward(context, pseudoResiduals, armIndex);

  banditsQueue.push(std::make_pair(armIndex, arm.getIndexScore()));
}

size_t LuapeGraphBuilderBanditPool::getArmWithHighestReward() const
{
  double bestReward = -DBL_MAX;
  size_t bestArm = (size_t)-1;
  for (size_t i = 0; i < arms.size(); ++i)
  {
    double reward = arms[i].getMeanReward();
    if (reward > bestReward)
      bestReward = reward, bestArm = i;
  }
  return bestArm;
}

void LuapeGraphBuilderBanditPool::displayInformation(ExecutionContext& context)
{
  std::multimap<double, size_t> armsByMeanReward;
  for (size_t i = 0; i < arms.size(); ++i)
    if (arms[i].node)
      armsByMeanReward.insert(std::make_pair(arms[i].getMeanReward(), i));

  size_t n = 10;
  size_t i = 1;
  for (std::multimap<double, size_t>::reverse_iterator it = armsByMeanReward.rbegin(); i < n && it != armsByMeanReward.rend(); ++it, ++i)
  {
    Arm& arm = arms[it->second];
    context.informationCallback(T("[") + String((int)i) + T("] r = ") + String(arm.getMeanReward())
      + T(" t = ") + String(arm.playedCount) + T(" -- ") + arm.node->toShortString());
  }

  if (armsByMeanReward.size())
  {
    Arm& bestArm = arms[armsByMeanReward.rbegin()->second];
    context.resultCallback(T("bestArmReward"), bestArm.getMeanReward());
    context.resultCallback(T("bestArmPlayCount"), bestArm.playedCount);
  }
  context.resultCallback(T("banditPoolSize"), arms.size());
}

void LuapeGraphBuilderBanditPool::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  for (size_t i = 0; i < arms.size(); ++i)
    if (arms[i].node)
      arms[i].getCache()->clearSamples(clearTrainingSamples, clearValidationSamples);
}

void LuapeGraphBuilderBanditPool::createBanditsQueue()
{
  banditsQueue = BanditsQueue();
  for (size_t i = 0; i < arms.size(); ++i)
    if (arms[i].node)
      banditsQueue.push(std::make_pair(i, arms[i].getIndexScore()));
}

size_t LuapeGraphBuilderBanditPool::createArm(ExecutionContext& context, const LuapeNodePtr& node)
{
  context.informationCallback(T("Create Arm ") + node->toShortString());
  jassert(node);
  size_t index;
  if (destroyedArmIndices.size())
  {
    index = destroyedArmIndices.back();
    destroyedArmIndices.pop_back();
  }
  else
  {
    index = arms.size();
    arms.push_back(Arm());
  }
  Arm& arm = arms[index];
  arm.node = node;
  nodeToArmIndex[node] = index;
  return index;
}

void LuapeGraphBuilderBanditPool::destroyArm(ExecutionContext& context, size_t index)
{
  context.informationCallback(T("Destroy arm ") + arms[index].node->toShortString());
  nodeToArmIndex[arms[index].node] = (size_t)-1;
  arms[index] = Arm();
  destroyedArmIndices.push_back(index);
  banditsQueue = BanditsQueue();
}

void LuapeGraphBuilderBanditPool::createNewArms(ExecutionContext& context, LuapeRPNGraphBuilderStatePtr state)
{
  if (state->isFinalState())
  {
    LuapeGraphPtr graph = state->getGraph();
    LuapeYieldNodePtr yield = graph->getLastNode().dynamicCast<LuapeYieldNode>();
    if (yield)
    {
      LuapeNodePtr node = yield->getArgument();
      if (nodeToArmIndex.find(node) == nodeToArmIndex.end())
        createArm(context, node);
    }
  }
  else
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable stateBackup;
      double reward;
      state->performTransition(context, actions->getElement(i), reward, &stateBackup);
     // context.enterScope(state->toShortString());
      createNewArms(context, state);
      //context.leaveScope();
      state->undoTransition(context, stateBackup);
    }
  }
}

/*
** LuapeGradientBoostingLearner
*/
LuapeGradientBoostingLearner::LuapeGradientBoostingLearner(LuapeGradientBoostingLossPtr loss, double learningRate, size_t maxBandits, size_t maxDepth)
  : loss(loss), learningRate(learningRate), maxBandits(maxBandits), maxDepth(maxDepth)
{
}

bool LuapeGradientBoostingLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeFunctionPtr& function)
{
  if (!loss->initialize(context, problem, function))
    return false;
  this->problem = problem;
  this->function = function;
  pool = new LuapeGraphBuilderBanditPool(maxBandits, maxDepth);
  graph = function->getGraph();
  context.enterScope(T("Creating initial arms"));
  pool->initialize(context, problem, graph);
  context.leaveScope(pool->getNumArms());
  return true;
}

bool LuapeGradientBoostingLearner::doLearningEpisode(ExecutionContext& context, const std::vector<ObjectPtr>& examples) const
{
  graph->clearSamples(true, false);
  pool->clearSamples(true, false);
  
  // 1- fill graph
  loss->setExamples(true, examples);

  // 2- compute graph outputs and compute loss derivatives
  context.enterScope(T("Computing predictions"));
  DenseDoubleVectorPtr predictions = function->computeSamplePredictions(context, true);
  context.leaveScope();
  context.enterScope(T("Computing pseudo-residuals"));
  double lossValue;
  DenseDoubleVectorPtr pseudoResiduals;
  loss->computeLoss(predictions, &lossValue, &pseudoResiduals);
  context.leaveScope();
  context.resultCallback(T("loss"), lossValue);

  context.resultCallback(T("predictions"), predictions);
  context.resultCallback(T("pseudoResiduals"), pseudoResiduals);

  // 3- play bandits
  if (pool->getNumArms() == 0)
  {
    context.errorCallback(T("No arms"));
    return false;
  }

  for (size_t t = 0; t < 10; ++t)
  {
    context.enterScope(T("Playing bandits iteration ") + String((int)t));
    for (size_t i = 0; i < pool->getNumArms(); ++i)
      pool->playArmWithHighestIndex(context, pseudoResiduals);
    pool->displayInformation(context);
    context.leaveScope();
  }

  // 4- select weak learner and find optimal weight
  LuapeNodePtr weakLearnerNode = pool->getArmNode(pool->getArmWithHighestReward());
  context.informationCallback(T("Weak learner: ") + weakLearnerNode->toShortString());
  LuapeNodeCachePtr weakLearnerCache = weakLearnerNode->getCache();
  BooleanVectorPtr weakPredictions = weakLearnerCache->getSamples(true).staticCast<BooleanVector>();
  double optimalWeight = loss->optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
  context.informationCallback(T("Optimal weight: ") + String(optimalWeight));
  
  // 5- add weak learner and associated weight to graph 
  graph->pushMissingNodes(context, weakLearnerNode);
  if (weakLearnerNode->getType() == booleanType)
  {
    // booleans are yielded directory
    graph->pushNode(context, new LuapeYieldNode(weakLearnerNode));
  }
  else
  {
    jassert(weakLearnerNode->getType()->isConvertibleToDouble());
    
    jassert(false); // todo: create stump
  }
  function->getVotes()->append(optimalWeight * learningRate);
  
  // 6- update arms
  pool->executeArm(context, pool->getArmWithHighestReward(), problem, graph, weakLearnerNode);

  context.informationCallback(T("Graph: ") + graph->toShortString());
  context.resultCallback(T("numArms"), pool->getNumArms());
  return true;
}


/*
class LuapeGraphBuilderBanditEnumerator
{
public:
  LuapeGraphBuilderBanditEnumerator(size_t maxDepth)
    : maxDepth(maxDepth) {}
  virtual ~LuapeGraphBuilderBanditEnumerator() {}

  void addNode(ExecutionContext& context, const LuapeGraphPtr& graph, size_t nodeIndex)
  {
    TypePtr nodeType = graph->getNodeType(nodeIndex);

    // accessor actions
    if (nodeType->inheritsFrom(objectClass))
    {
      std::vector<size_t> arguments(1, nodeIndex);
      size_t nv = nodeType->getNumMemberVariables();
      for (size_t j = 0; j < nv; ++j)
        res->append(new LuapeFunctionNode(getVariableFunction(j), arguments));
    }
    
    // function actions
    for (size_t i = 0; i < problem->getNumFunctions(); ++i)
    {
      FunctionPtr function = problem->getFunction(i);
      std::vector<size_t> arguments;
      enumerateFunctionActionsRecursively(function, arguments, res);
    }

    // yield actions
    for (size_t i = 0; i < n; ++i)
      if (graph->getNodeType(i) == booleanType)
        res->append(new LuapeYieldNode(i));
    return res;
  }

protected:
  std::map<LuapeNodeKey, LuapeNodeCachePtr> cache;
};
*/