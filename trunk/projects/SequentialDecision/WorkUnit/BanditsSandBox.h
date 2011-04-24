/*-----------------------------------------.---------------------------------.
| Filename: BanditsSandBox.h               | Bandits Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

# include "../Problem/BanditsDecisionProblem.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class BanditStatistics : public Object
{
public:
  BanditStatistics() : playedCount(0), sumOfRewards(0.0), sumOfSquaredRewards(0.0) {}

  void update(double reward)
  {
    ++playedCount;
    sumOfRewards += reward;
    sumOfSquaredRewards += reward * reward;
  }

  size_t getPlayedCount() const
    {return playedCount;}

  double getAverageReward() const
    {jassert(playedCount); return sumOfRewards / playedCount;}

private:
  friend class BanditStatisticsClass;

  size_t playedCount;
  double sumOfRewards;
  double sumOfSquaredRewards;
};

typedef ReferenceCountedObjectPtr<BanditStatistics> BanditStatisticsPtr;

class BanditsPolicy : public Object
{
public:
  virtual void initialize(size_t numBandits)
  {
    timeStep = 0;
    banditStatistics.resize(numBandits);
    for (size_t i = 0; i < numBandits; ++i)
      banditStatistics[i] = new BanditStatistics();
  }

  size_t selectNextBandit()
    {return selectBandit(timeStep, banditStatistics);}

  void updatePolicy(size_t banditNumber, double reward)
  {
    jassert(banditNumber < banditStatistics.size());
    ++timeStep;
    banditStatistics[banditNumber]->update(reward);
  }

protected:
  friend class BanditsPolicyClass;

  size_t timeStep;
  std::vector<BanditStatisticsPtr> banditStatistics;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) = 0;
};

class IndexBasedBanditsPolicy : public BanditsPolicy
{
protected:
  virtual bool playEachMachineOnceAtInitialization() const = 0;
  virtual double computeBanditIndex(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const = 0;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (playEachMachineOnceAtInitialization() && timeStep < numBandits)
      return timeStep; // play each bandit once

    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
    for (size_t i = 0; i < numBandits; ++i)
    {
      double score = computeBanditIndex(i, timeStep, banditStatistics);
      if (score > bestScore)
        bestBandit = i, bestScore = score;
    }
    return bestBandit;
  }
};

class UCB1BanditsPolicy : public IndexBasedBanditsPolicy
{
protected:
  virtual bool playEachMachineOnceAtInitialization() const
    {return true;}

  virtual double computeBanditIndex(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getAverageReward() + sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }
};

// TODO:
// Finish UCB2BanditsPolicy
// UCB1TunedBanditsPolicy
// UCB1NormalBanditsPolicy
// EpsilonGreedyBanditsPolicy

class UCB2BanditsPolicy : public IndexBasedBanditsPolicy
{
public:
  UCB2BanditsPolicy(double alpha = 1.0) : alpha(alpha) {}
 
  virtual void initialize(size_t numBandits)
  {
    IndexBasedBanditsPolicy::initialize(numBandits);
    episodeCounts.clear();
    episodeCounts.resize(numBandits, 0);
  }

protected:
  friend class UCB2BanditsPolicyClass;

  double alpha;
  std::vector<size_t> episodeCounts;

  virtual bool playEachMachineOnceAtInitialization() const
    {return true;}

  virtual double computeBanditIndex(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tauEpisodeCount = tau(episodeCounts[banditNumber]);
    double e = 1.0; // FIXME: what is e ????
    return statistics->getAverageReward() + sqrt((1.0 + alpha) * log(e * timeStep / tauEpisodeCount) / (2.0 * tauEpisodeCount));
  }

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    // FIXME: implement this function !!!
    size_t numBandits = banditStatistics.size();
    if (playEachMachineOnceAtInitialization() && timeStep < numBandits)
      return timeStep; // play each bandit once

    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
    for (size_t i = 0; i < numBandits; ++i)
    {
      double score = computeBanditIndex(i, timeStep, banditStatistics);
      if (score > bestScore)
        bestBandit = i, bestScore = score;
    }
    return bestBandit;
  }

  size_t tau(size_t count) const
    {return (size_t)ceil(pow(1 + alpha, (double)count));}
};

/*
** Sand Box
*/
class BanditsSandBox : public WorkUnit
{
public:
  BanditsSandBox() {}
 
  virtual Variable run(ExecutionContext& context)
  {
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
