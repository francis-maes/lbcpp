/*-----------------------------------------.---------------------------------.
| Filename: LuapeGradientBoosting.h        | Luape Gradient Boosting         |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_GRADIENT_BOOSTING_H_

# include "LuapeFunction.h"
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

  const LuapeNodeCachePtr& getArmCache(size_t index) const
    {jassert(index < arms.size()); return arms[index].getCache();}

  void initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeGraphPtr& graph);
  void executeArm(ExecutionContext& context, size_t armIndex,  const LuapeProblemPtr& problem, const LuapeGraphPtr& graph);

  // train the weak learner on 50% of data and evaluate on the other 50% of data
  double sampleReward(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals, size_t armIndex) const;
  void playArmWithHighestIndex(ExecutionContext& context, const DenseDoubleVectorPtr& pseudoResiduals);

  size_t getArmWithHighestReward() const;

  void displayInformation(ExecutionContext& context);

  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

protected:
  size_t maxSize;
  size_t maxDepth;

  struct Arm
  {
    Arm(LuapeNodePtr node = LuapeNodePtr())
      : playedCount(0), rewardSum(0.0), node(node) {}

    size_t playedCount;
    double rewardSum;
    LuapeNodePtr node;
    LuapeNodeKey key;
    String description;

    const LuapeNodeCachePtr& getCache() const
      {return node->getCache();}

    double getIndexScore() const
      {return playedCount ? rewardSum + 2.0 / (double)playedCount : DBL_MAX;}

    double getMeanReward() const
      {return playedCount ? rewardSum / (double)playedCount : 0.0;}
  };

  std::vector<Arm> arms;
  std::vector<size_t> destroyedArmIndices;

  typedef std::map<LuapeNodeKey, size_t> KeyToArmIndexMap;
  KeyToArmIndexMap keyToArmIndex;

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
  size_t createArm(ExecutionContext& context, const LuapeNodeKey& key, const LuapeNodePtr& node, const String& description);
  void destroyArm(ExecutionContext& context, size_t index);
  void createNewArms(ExecutionContext& context, LuapeRPNGraphBuilderStatePtr state);
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderBanditPool> LuapeGraphBuilderBanditPoolPtr;

class LuapeGradientBoostingLoss : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeFunctionPtr& function) = 0;
  virtual void setExamples(bool isTrainingData, const std::vector<ObjectPtr>& data) = 0;
  virtual DenseDoubleVectorPtr computePseudoResiduals(const DenseDoubleVectorPtr& predictions) const = 0;
  virtual double optimizeWeightOfWeakLearner(const DenseDoubleVectorPtr& predictions, const BooleanVectorPtr& weakPredictions) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLoss> LuapeGradientBoostingLossPtr;

class LuapeGradientBoostingLearner : public Object
{
public:
  LuapeGradientBoostingLearner(LuapeGradientBoostingLossPtr loss, double learningRate, size_t maxBandits, size_t maxDepth);

  bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeFunctionPtr& function);
  bool doLearningEpisode(ExecutionContext& context, const std::vector<ObjectPtr>& examples) const;

protected:
  LuapeGradientBoostingLossPtr loss;
  double learningRate;
  size_t maxBandits;
  size_t maxDepth;

  LuapeGraphBuilderBanditPoolPtr pool;

  LuapeProblemPtr problem;
  LuapeFunctionPtr function;
  LuapeGraphPtr graph;
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLearner> LuapeGradientBoostingLearnerPtr;


///////////////////////////////////////////

class RankingGradientBoostingLoss : public LuapeGradientBoostingLoss
{
public:
  RankingGradientBoostingLoss(RankingLossFunctionPtr rankingLoss = RankingLossFunctionPtr())
    : rankingLoss(rankingLoss) {}

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeFunctionPtr& function)
  {
    this->problem = problem;
    this->function = function;
    this->graph = function->getGraph();
    return true;
  }

  virtual void setExamples(bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    graph->clearSamples(isTrainingData, !isTrainingData);
    for (size_t i = 0; i < data.size(); ++i)
      addRankingExampleToGraph(isTrainingData, data[i].staticCast<Pair>());
    if (isTrainingData)
      trainingData = *(std::vector<PairPtr>* )&data;
    else
      validationData = *(std::vector<PairPtr>* )&data;
  }

  void addRankingExampleToGraph(bool isTrainingData, const PairPtr& rankingExample) const
  {
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();
    size_t firstIndex = graph->getNumSamples(isTrainingData);
    graph->resizeSamples(isTrainingData, firstIndex + n);
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<Variable> sample(1, alternatives->getElement(i));
      graph->setSample(true, firstIndex + i, sample);
    }
  }

  virtual DenseDoubleVectorPtr computePseudoResiduals(const DenseDoubleVectorPtr& predictions) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(predictions->getNumValues(), 0.0);

    size_t index = 0;
    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      size_t n = trainingData[i]->getFirst().getObjectAndCast<Container>()->getNumElements();
      DenseDoubleVectorPtr costs = trainingData[i]->getSecond().getObjectAndCast<DenseDoubleVector>();

      DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
      memcpy(scores->getValuePointer(0), predictions->getValuePointer(index), sizeof (double) * n);

      double lossValue = 0.0;
      DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(n, 0.0);
      rankingLoss->computeRankingLoss(scores, costs, &lossValue, &lossGradient, 1.0);
      jassert(lossGradient);
      
      memcpy(res->getValuePointer(index), lossGradient->getValuePointer(0), sizeof (double) * n);
      index += n;
    }

    res->multiplyByScalar(-1.0);
    return res;
  }

  virtual double optimizeWeightOfWeakLearner(const DenseDoubleVectorPtr& predictions, const BooleanVectorPtr& weakPredictions) const
  {
    return 1.0; // FIXME !! 
  }

protected:
  RankingLossFunctionPtr rankingLoss;

  LuapeProblemPtr problem;
  LuapeFunctionPtr function;
  LuapeGraphPtr graph;

  std::vector<PairPtr> trainingData;
  std::vector<PairPtr> validationData;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRADIENT_BOOSTING_H_
