/*-----------------------------------------.---------------------------------.
| Filename: LuapeBanditPoolGBLearner.h     | Luape Bandit based Gradient     |
| Author  : Francis Maes                   |  Boosting learner               |
| Started : 19/11/2011 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_BANDIT_POOL_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_BANDIT_POOL_GRADIENT_BOOSTING_H_

# include "LuapeGradientBoostingLearner.h"
# include <queue> // for priority queue in bandits pool

namespace lbcpp
{

class LuapeGraphBuilderBanditPool : public Object
{
public:
  LuapeGraphBuilderBanditPool(size_t maxSize, size_t maxDepth);

  size_t getNumArms() const
    {return arms.size();}

  const LuapeNodePtr& getArmNode(size_t index) const
    {jassert(index < arms.size()); return arms[index].node;}

  void setArmNode(size_t index, const LuapeNodePtr& node)
    {jassert(index < arms.size()); arms[index].node = node;}

  LuapeNodeCachePtr getArmCache(size_t index) const
    {jassert(index < arms.size()); return arms[index].getCache();}

  void initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph);
  void executeArm(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode);

  void playArmWithHighestIndex(ExecutionContext& context, const LuapeGreedyStructureLearnerPtr& graphLearner);

  size_t sampleArmWithHighestReward(ExecutionContext& context) const;

  void displayInformation(ExecutionContext& context);
  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  size_t createArm(ExecutionContext& context, const LuapeNodePtr& node);
  void destroyArm(ExecutionContext& context, size_t index);

protected:
  size_t maxSize;
  size_t maxDepth;

  struct Arm
  {
    Arm(LuapeNodePtr node = LuapeNodePtr())
      : playedCount(0), rewardSum(0.0), node(node) {}

    size_t playedCount;
    double rewardSum;

    void reset()
      {playedCount = 0; rewardSum = 0.0;}

    LuapeNodePtr node;

    LuapeNodeCachePtr getCache() const
      {return node ? node->getCache() : LuapeNodeCachePtr();}

    double getIndexScore() const
      {return playedCount ? (rewardSum + 2.0) / (double)playedCount : DBL_MAX;}

    double getMeanReward() const
      {return playedCount ? rewardSum / (double)playedCount : 0.0;}
  };

  std::vector<Arm> arms;

  struct BanditScoresComparator
  {
    bool operator()(const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
    {
      if (left.second != right.second)
        return left.second < right.second;
      else
        return left.first < right.first;
    }
  };
  
  typedef std::priority_queue<std::pair<size_t, double>, std::vector<std::pair<size_t, double> >, BanditScoresComparator  > BanditsQueue;
  BanditsQueue banditsQueue;

  void createBanditsQueue();
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderBanditPool> LuapeGraphBuilderBanditPoolPtr;


/*
** LuapeBanditPoolWeakLearner
*/
class LuapeBanditPoolWeakLearner : public LuapeWeakLearner
{
public:
  LuapeBanditPoolWeakLearner(size_t maxBandits, size_t maxDepth)
    : maxBandits(maxBandits), maxDepth(maxDepth) {}  

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
  {
    pool = new LuapeGraphBuilderBanditPool(maxBandits, maxDepth);
    pool->clearSamples(true, true);
    pool->initialize(context, problem, function->getGraph());
    return true;
  }

  // gradient boosting
  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeGreedyStructureLearnerPtr& structureLearner) const
  {
    if (pool->getNumArms() == 0)
    {
      context.errorCallback(T("No arms"));
      return LuapeNodePtr();
    }
    
    // play arms
    for (size_t t = 0; t < 10; ++t)
    {
      context.enterScope(T("Playing bandits iteration ") + String((int)t));
      for (size_t i = 0; i < pool->getNumArms(); ++i)
        pool->playArmWithHighestIndex(context, structureLearner);
      pool->displayInformation(context);
      context.leaveScope();
    }

    // select best arm
    size_t armIndex = pool->sampleArmWithHighestReward(context);
    if (armIndex == (size_t)-1)
    {
      context.errorCallback(T("Could not select best arm"));
      return LuapeNodePtr();
    }

    return pool->getArmNode(armIndex);
  }
  
  virtual void update(ExecutionContext& context, const LuapeGreedyStructureLearnerPtr& structureLearner, LuapeNodePtr weakLearner)
  {
    pool->executeArm(context, structureLearner->getProblem(), structureLearner->getGraph(), weakLearner);
    context.resultCallback(T("numArms"), pool->getNumArms());
  }

protected:
  size_t maxBandits;
  size_t maxDepth;
  LuapeGraphBuilderBanditPoolPtr pool;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_BANDIT_POOL_GRADIENT_BOOSTING_H_
