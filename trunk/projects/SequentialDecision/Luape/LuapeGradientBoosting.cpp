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

class LuapeGraphBuilderEnumerator
{
public:
  LuapeGraphBuilderEnumerator(LuapeGraphBuilderBanditPoolPtr pool, size_t maxDepth)
    : pool(pool), maxDepth(maxDepth) {}
  virtual ~LuapeGraphBuilderEnumerator() {}

  void clearCache()
  {
    keyToNodes.clear();
    nodeToKeys.clear();
  }

  void addSubNodesToCache(ExecutionContext& context, const LuapeNodePtr& node)
  {
    LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
    if (functionNode)
    {
      std::vector<LuapeNodePtr> arguments = functionNode->getArguments();
      for (size_t i = 0; i < arguments.size(); ++i)
        addNodeToCache(context, arguments[i]);
    }
  }

  // return true if it is a new node
  bool addNodeToCache(ExecutionContext& context, const LuapeNodePtr& node)
  {
    NodeToKeyMap::const_iterator it = nodeToKeys.find(node);
    if (it != nodeToKeys.end())
      return false; // we already know about this node

    // compute node key
    node->updateCache(context, true);
    BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();

    KeyToNodeMap::iterator it2 = keyToNodes.find(key);
    if (it2 == keyToNodes.end())
    {
      // this is a new node equivalence class
      nodeToKeys[node] = key;
      keyToNodes[key] = node;
      addSubNodesToCache(context, node);
      return true;
    }
    else
    {
      // existing node equivalence class
      //  see if new node is better than previous one to represent the class
      LuapeNodePtr previousNode = it2->second;
      if (node->getDepth() < previousNode->getDepth())
      {
        it2->second = node;
        context.informationCallback(T("Change computation of ") + previousNode->toShortString() + T(" into ") + node->toShortString());
        LuapeFunctionNodePtr sourceFunctionNode = node.dynamicCast<LuapeFunctionNode>();
        LuapeFunctionNodePtr targetFunctionNode = previousNode.dynamicCast<LuapeFunctionNode>();
        if (sourceFunctionNode && targetFunctionNode)
          sourceFunctionNode->clone(context, targetFunctionNode);
        addSubNodesToCache(context, node);
      }
      nodeToKeys[node] = it2->first;
      return false;
    }
  }

  void updateArms(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
  {
    clearCache();
   
    // fill map with keys of already existing nodes
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (!node.isInstanceOf<LuapeYieldNode>())
        addNodeToCache(context, node);
    }

    // fill map with existing arms
    for (size_t i = 0; i < pool->getNumArms(); ++i)
    {
      LuapeNodePtr node = pool->getArmNode(i);
      if (node)
        addNodeToCache(context, node);
    }
    
    enumerateNewArms(context, problem, graph, newNode);
  }

  void enumerateNewArms(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
  {
    LuapeGraphBuilderStatePtr builder = new LuapeGraphBuilderState(problem, graph->cloneAndCast<LuapeGraph>(), maxDepth);
    //builder->performTransition(context, LuapeGraphBuilderAction::push(newNode));
    enumerateNewArmsRecursively(context, builder);
  }

  void enumerateNewArmsRecursively(ExecutionContext& context, const LuapeGraphBuilderStatePtr& state)
  {
    if (state->isFinalState())
    {
      LuapeYieldNodePtr yieldNode = state->getGraph()->getLastNode().dynamicCast<LuapeYieldNode>();
      if (yieldNode)
      {
        LuapeNodePtr node = yieldNode->getArgument();
        if (addNodeToCache(context, node))
          pool->createArm(context, node);
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
        //context.enterScope(state->toShortString());
        enumerateNewArmsRecursively(context, state);
        //context.leaveScope();
        state->undoTransition(context, stateBackup);
      }
    }
  }

protected:
  LuapeGraphBuilderBanditPoolPtr pool;
  size_t maxDepth;

  typedef std::map<BinaryKeyPtr, LuapeNodePtr, ObjectComparator> KeyToNodeMap;
  typedef std::map<LuapeNodePtr, BinaryKeyPtr> NodeToKeyMap;

  KeyToNodeMap keyToNodes;
  NodeToKeyMap nodeToKeys;
};

/*
** LuapeGraphBuilderBanditPool
*/
LuapeGraphBuilderBanditPool::LuapeGraphBuilderBanditPool(size_t maxSize, size_t maxDepth)
  : maxSize(maxSize), maxDepth(maxDepth)
{
}

void LuapeGraphBuilderBanditPool::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph)
{
  LuapeGraphBuilderEnumerator enumerator(this, maxDepth);
  context.enterScope(T("Initialize arms"));
  for (size_t i = 0; i < graph->getNumNodes(); ++i)
    enumerator.updateArms(context, problem, graph, graph->getNode(i));
  createBanditsQueue();
  context.leaveScope();
}

void LuapeGraphBuilderBanditPool::executeArm(ExecutionContext& context, size_t armIndex,  const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
{
  // update arms
  arms[armIndex].reset();

  LuapeGraphBuilderEnumerator enumerator(this, maxDepth);
  context.enterScope(T("Update arms"));
  enumerator.updateArms(context, problem, graph, newNode);
  createBanditsQueue();
  context.leaveScope();

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

    if (validationPositive.getCount() || validationNegative.getCount())
      meanSquareError /= validationPositive.getCount() + validationNegative.getCount();
    
    return 1.0 - meanSquareError;
  }
}

void LuapeGraphBuilderBanditPool::playArmWithHighestIndex(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals)
{
  if (banditsQueue.size())
  {
    size_t armIndex = banditsQueue.top().first;
    banditsQueue.pop();
    
    Arm& arm = arms[armIndex];
    ++arm.playedCount;
    arm.rewardSum += sampleReward(context, pseudoResiduals, armIndex);

    banditsQueue.push(std::make_pair(armIndex, arm.getIndexScore()));
  }
}

size_t LuapeGraphBuilderBanditPool::sampleArmWithHighestReward(ExecutionContext& context) const
{
  double bestReward = -DBL_MAX;
  size_t bestArm = (size_t)-1;
  std::vector<size_t> bests;
  for (size_t i = 0; i < arms.size(); ++i)
  {
    double reward = arms[i].getMeanReward();
    if (reward >= bestReward)
    {
      if (reward > bestReward)
      {
        bestReward = reward;
        bests.clear();
      }
      bests.push_back(i);
    }
  }
  RandomGeneratorPtr random = context.getRandomGenerator();
  if (bests.empty())
    return random->sampleSize(arms.size());
  else
    return bests[random->sampleSize(bests.size())];
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
  return index;
}

void LuapeGraphBuilderBanditPool::destroyArm(ExecutionContext& context, size_t index)
{
  context.informationCallback(T("Destroy arm ") + arms[index].node->toShortString());
  arms[index] = Arm();
  destroyedArmIndices.push_back(index);
  banditsQueue = BanditsQueue();
}

/*
** LuapeGradientBoostingLearner
*/
LuapeGradientBoostingLearner::LuapeGradientBoostingLearner(LuapeProblemPtr problem, double learningRate, size_t maxBandits, size_t maxDepth)
  : problem(problem), learningRate(learningRate), maxBandits(maxBandits), maxDepth(maxDepth)
{
}

bool LuapeGradientBoostingLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  if (!problem->getObjective()->initialize(context, problem, function))
    return false;
  this->problem = problem;
  this->function = function;
  pool = new LuapeGraphBuilderBanditPool(maxBandits, maxDepth);
  graph = function->getGraph();
  return true;
}

bool LuapeGradientBoostingLearner::doLearningEpisode(ExecutionContext& context, const std::vector<ObjectPtr>& examples) const
{
  graph->clearSamples(true, false);
  pool->clearSamples(true, false);
  
  // 1- fill graph
  LuapeObjectivePtr objective = problem->getObjective();
  objective->setExamples(true, examples);
  if (!pool->getNumArms())
    pool->initialize(context, problem, graph);

  // 2- compute graph outputs and compute loss derivatives
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
  size_t armIndex = pool->sampleArmWithHighestReward(context);
  if (armIndex == (size_t)-1)
  {
    context.errorCallback(T("Could not select best arm"));
    return false;
  }
  LuapeNodePtr weakLearnerNode = pool->getArmNode(armIndex);
  context.informationCallback(T("Weak learner: ") + weakLearnerNode->toShortString());
  LuapeNodeCachePtr weakLearnerCache = weakLearnerNode->getCache();
  BooleanVectorPtr weakPredictions = weakLearnerCache->getSamples(true).staticCast<BooleanVector>();
  double optimalWeight = objective->optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
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
  pool->executeArm(context, armIndex, problem, graph, weakLearnerNode);

  context.informationCallback(T("Graph: ") + graph->toShortString());
  context.resultCallback(T("numArms"), pool->getNumArms());
  return true;
}
