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

  void playArmWithHighestIndex(ExecutionContext& context, const LuapeGraphLearnerPtr& graphLearner);

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

class LuapeBanditPoolGBLearner : public LuapeGradientBoostingLearner
{
public:
  LuapeBanditPoolGBLearner(double learningRate, size_t maxBandits, size_t maxDepth);

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);

  // gradient boosting
  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context, const DenseDoubleVectorPtr& predictions) const;
  virtual bool addWeakLearnerToGraph(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, LuapeNodePtr completion);

protected:
  size_t maxBandits;
  LuapeGraphBuilderBanditPoolPtr pool;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_BANDIT_POOL_GRADIENT_BOOSTING_H_
