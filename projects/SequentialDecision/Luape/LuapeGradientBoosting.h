/*-----------------------------------------.---------------------------------.
| Filename: LuapeGradientBoosting.h        | Luape Gradient Boosting         |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_GRADIENT_BOOSTING_H_

# include "LuapeInference.h"
# include "LuapeGraphBuilder.h"
# include "LuapeProblem.h"

# include <queue> // for priority queue in bandits pool
# include <lbcpp/Learning/LossFunction.h> // for ranking loss

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
  void executeArm(ExecutionContext& context, size_t armIndex,  const LuapeProblemPtr& problem, const LuapeGraphPtr& graph, const LuapeNodePtr& newNode);

  // train the weak learner on 50% of data and evaluate on the other 50% of data
  double sampleReward(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals, size_t armIndex) const;
  void playArmWithHighestIndex(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals);

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
  std::vector<size_t> destroyedArmIndices;

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

class LuapeGradientBoostingLearner : public Object
{
public:
  LuapeGradientBoostingLearner(LuapeProblemPtr problem, double learningRate, size_t maxBandits, size_t maxDepth);
  LuapeGradientBoostingLearner() : learningRate(0.0), maxBandits(0), maxDepth(maxDepth) {}

  bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);
  bool doLearningEpisode(ExecutionContext& context, const std::vector<ObjectPtr>& examples) const;

protected:
  LuapeProblemPtr problem;
  double learningRate;
  size_t maxBandits;
  size_t maxDepth;

  LuapeGraphBuilderBanditPoolPtr pool;

  LuapeInferencePtr function;
  LuapeGraphPtr graph;
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLearner> LuapeGradientBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRADIENT_BOOSTING_H_
