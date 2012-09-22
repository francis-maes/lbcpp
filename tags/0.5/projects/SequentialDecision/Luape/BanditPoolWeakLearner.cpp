/*-----------------------------------------.---------------------------------.
| Filename: BanditPoolWeakLearner.cpp      | Luape Bandit based Gradient     |
| Author  : Francis Maes                   |  Boosting learner               |
| Started : 19/11/2011 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "BanditPoolWeakLearner.h"
using namespace lbcpp;

#if 0

// FIXME: there are two redundant node keys maps: one in the enumerator and the other inside the graph builder
class LuapeGraphBuilderEnumerator
{
public:
  LuapeGraphBuilderEnumerator(LuapeGraphBuilderBanditPoolPtr pool, size_t maxDepth)
    : pool(pool), maxDepth(maxDepth), nodeKeys(new LuapeNodeKeysMap()) {}
  virtual ~LuapeGraphBuilderEnumerator() {}

  void updateArms(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
  {
    nodeKeys->clear();
   
    // fill map with keys of already existing nodes
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      LuapeNodePtr node = graph->getNode(i);
      if (!node.isInstanceOf<LuapeYieldNode>())
        nodeKeys->addNodeToCache(context, node);
    }

    // fill map with existing arms
    for (size_t i = 0; i < pool->getNumArms(); ++i)
    {
      LuapeNodePtr node = pool->getArmNode(i);
      if (node)
        nodeKeys->addNodeToCache(context, node);
    }
    
    enumerateNewArms(context, problem, graph, newNode);
  }

  void enumerateNewArms(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
  {
    LuapeNodeBuilderStatePtr builder = new LuapeNodeBuilderState(graph->cloneAndCast<LuapeGraph>(), new LuapeGraphBuilderTypeSearchSpace(problem, maxDepth));
    //builder->performTransition(context, LuapeNodeBuilderAction::push(newNode));
    enumerateNewArmsRecursively(context, builder);
  }

  void enumerateNewArmsRecursively(ExecutionContext& context, const LuapeNodeBuilderStatePtr& state)
  {
    if (state->isFinalState())
    {
      LuapeYieldNodePtr yieldNode = state->getGraph()->getLastNode().dynamicCast<LuapeYieldNode>();
      if (yieldNode)
      {
        LuapeNodePtr node = yieldNode->getArgument();
        if (nodeKeys->addNodeToCache(context, node))
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

  LuapeNodeKeysMapPtr nodeKeys;
};

/*
** LuapeGraphBuilderBanditPool
*/
LuapeGraphBuilderBanditPool::LuapeGraphBuilderBanditPool(size_t maxSize, size_t maxDepth)
  : maxSize(maxSize), maxDepth(maxDepth)
{
}

void LuapeGraphBuilderBanditPool::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  jassert(false); // broken
/*  LuapeGraphBuilderEnumerator enumerator(this, maxDepth);
  context.enterScope(T("Initialize arms"));
  for (size_t i = 0; i < graph->getNumNodes(); ++i)
    enumerator.updateArms(context, problem, graph, graph->getNode(i));
  createBanditsQueue();
  context.leaveScope();*/
}

void LuapeGraphBuilderBanditPool::executeArm(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode)
{
  LuapeGraphBuilderEnumerator enumerator(this, maxDepth);
  context.enterScope(T("Update arms"));
  enumerator.updateArms(context, problem, graph, newNode);
  createBanditsQueue();
  context.leaveScope();

  context.informationCallback(String((int)arms.size()) + T(" arms"));
}

void LuapeGraphBuilderBanditPool::playArmWithHighestIndex(ExecutionContext& context, const BoostingLearnerPtr& graphLearner, const IndexSetPtr& examples)
{
  if (banditsQueue.size())
  {
    size_t armIndex = banditsQueue.top().first;
    banditsQueue.pop();
    
    Arm& arm = arms[armIndex];
    ++arm.playedCount;
    LuapeNodePtr node = arms[armIndex].node;
    jassert(false); // FIXME
    //arm.rewardSum += computeWeakObjectiveWithEventualStump(context, graphLearner, node, examples);
    banditsQueue.push(std::make_pair(armIndex, arm.getIndexScore()));
  }
}

size_t LuapeGraphBuilderBanditPool::sampleArmWithHighestReward(ExecutionContext& context) const
{
  double bestReward = -DBL_MAX;
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
      + T(" t = ") + String((int)arm.playedCount) + T(" -- ") + arm.node->toShortString());
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
  size_t index = arms.size();
  arms.push_back(Arm());
  Arm& arm = arms[index];
  arm.node = node;
  return index;
}

void LuapeGraphBuilderBanditPool::destroyArm(ExecutionContext& context, size_t index)
{
  context.informationCallback(T("Destroy arm ") + arms[index].node->toShortString());
  arms[index] = Arm();
  banditsQueue = BanditsQueue();
}

#endif // 0
