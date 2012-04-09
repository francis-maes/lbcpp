/*-----------------------------------------.---------------------------------.
| Filename: MCBanditPool.h                 | Monte Carlo Bandit Pool         |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2012 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_BANDIT_POOL_H_
# define LBCPP_LUAPE_MC_BANDIT_POOL_H_

# include <queue>

namespace lbcpp
{

class MCBanditPool : public Object, public ExecutionContextCallback
{
public:
  MCBanditPool(const FunctionPtr& objective, double explorationCoefficient, bool useMultiThreading = false) 
    : objective(objective), explorationCoefficient(explorationCoefficient), useMultiThreading(useMultiThreading) {}
  MCBanditPool() : explorationCoefficient(0.0) {}

  size_t getNumArms() const
    {return arms.size();}

  void reserveArms(size_t count);
  size_t createArm(const Variable& parameter);
  void destroyArm(size_t index);
  const Variable& getArmParameter(size_t index) const;
  void setArmParameter(size_t index, const Variable& parameter);

  size_t selectAndPlayArm(ExecutionContext& context);
  size_t sampleArmWithHighestReward(ExecutionContext& context) const;
  void observeReward(size_t index, double reward);

  void displayInformation(ExecutionContext& context, size_t numBestArms = 10);

  void play(ExecutionContext& context, size_t numTimeSteps, bool showProgression = true);
  void playIterations(ExecutionContext& context, size_t numIterations, size_t stepsPerIteration);

protected:
  friend class MCBanditPoolClass;

  FunctionPtr objective; // samples rewards in range [0,1]
  double explorationCoefficient;
  bool useMultiThreading;

  struct Arm
  {
    Arm() : playedCount(0), rewardSum(0.0) {}

    size_t playedCount;
    double rewardSum;

    Variable parameter;

    void observe(double reward)
      {++playedCount; rewardSum += reward;}

    void reset()
      {playedCount = 0; rewardSum = 0.0;}

    double getMeanReward() const
      {return playedCount ? rewardSum / (double)playedCount : 0.0;}
  };

  std::vector<Arm> arms;

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result, const ExecutionTracePtr& trace);
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

  std::map<Variable, size_t> currentlyPlayedArms;

  size_t getNumCurrentlyPlayedArms() const
    {return currentlyPlayedArms.size();}
};

typedef ReferenceCountedObjectPtr<MCBanditPool> MCBanditPoolPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_BANDIT_POOL_H_
