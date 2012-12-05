/*-----------------------------------------.---------------------------------.
| Filename: BanditPool.h                   | Bandit Pool                     |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2012 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_BANDIT_POOL_H_
# define ML_BANDIT_POOL_H_

# include <oil/Core.h>
# include "predeclarations.h"
# include <ml/Objective.h>
# include <queue>

namespace lbcpp
{

class BanditPool : public Object, public ExecutionContextCallback
{
public:
  BanditPool(const StochasticObjectivePtr& objective, double explorationCoefficient, bool optimizeMax = false, bool useMultiThreading = false);
  BanditPool();

  size_t getNumArms() const
    {return arms.size();}

  void reserveArms(size_t count);
  size_t createArm(const ObjectPtr& object);
  void destroyArm(size_t index);
  
  const ObjectPtr& getArmObject(size_t index) const;
  double getArmMeanObjective(size_t index) const;
  double getArmMeanReward(size_t index) const;
  size_t getArmPlayedCount(size_t index) const;

  void setArmObject(size_t index, const ObjectPtr& object);

  size_t selectAndPlayArm(ExecutionContext& context);
  size_t sampleArmWithHighestReward(ExecutionContext& context) const;
  void observeObjective(size_t index, double objective);

  void displayArmInformation(ExecutionContext& context, size_t order, size_t armIndex) const;
  void displayInformation(ExecutionContext& context, size_t numBestArms = 15, size_t numWorstArms = 5) const;
  void displayAllArms(ExecutionContext& context);

  void getArmsOrder(std::vector< std::pair<size_t, double> >& res) const; // from best to worst w.r.t. mean objective value

  void play(ExecutionContext& context, size_t numTimeSteps, bool showProgression = true);
  void playIterations(ExecutionContext& context, size_t numIterations, size_t stepsPerIteration);

protected:
  friend class BanditPoolClass;

  StochasticObjectivePtr objective;
  double explorationCoefficient;
  bool optimizeMax;
  bool useMultiThreading;

  struct Arm
  {
    Arm() : playedCount(0), objectiveValueSum(0.0), objectiveValueBest(0.0), rewardSum(0.0), rewardMin(1.0), rewardMax(0.0) {}

    size_t playedCount;
    double objectiveValueSum;
    double objectiveValueBest;
    double rewardSum;
    double rewardMin;
    double rewardMax;

    ObjectPtr object;

    void observe(double objectiveValue, double reward)
    {
      ++playedCount;
      objectiveValueSum += objectiveValue;
      rewardSum += reward;
      rewardMin = juce::jmin(rewardMin, reward);
      rewardMax = juce::jmax(rewardMax, reward);
    }

    void reset()
      {playedCount = 0; rewardSum = 0.0;}

    double getMeanReward() const
      {return playedCount ? rewardSum / (double)playedCount : 0.0;}

    double getMeanObjectiveValue() const
      {return playedCount ? objectiveValueSum / (double)playedCount : 0.0;}
  };

  std::vector<Arm> arms;

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const ObjectPtr& result, const ExecutionTracePtr& trace);
  double getIndexScore(Arm& arm) const;

  struct ArmScoreComparator
  {
    bool operator()(const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
    {
      if (left.second != right.second)
        return left.second < right.second;
      else
        return left.first < right.first;
    }
  };
  
  typedef std::priority_queue<std::pair<size_t, double>, std::vector<std::pair<size_t, double> >, ArmScoreComparator> ArmsQueue;
  ArmsQueue queue;

  void pushArmIntoQueue(size_t index, double score);
  int popArmFromQueue();

  size_t getNumCurrentlyPlayedArms() const
    {return arms.size() - queue.size();}
};

typedef ReferenceCountedObjectPtr<BanditPool> BanditPoolPtr;

}; /* namespace lbcpp */

#endif // !ML_BANDIT_POOL_H_
