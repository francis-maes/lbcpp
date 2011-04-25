/*-----------------------------------------.---------------------------------.
| Filename: BanditsSandBox.h               | Bandits Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

# include "../Problem/DiscreteBanditDecisionProblem.h"
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

  double getRewardMean() const
    {jassert(playedCount); return sumOfRewards / playedCount;}

  double getSquaredRewardMean() const
    {jassert(playedCount); return sumOfSquaredRewards / playedCount;}

  double getRewardVariance() const
    {double ar = getRewardMean(); return getSquaredRewardMean() - ar * ar;}

private:
  friend class BanditStatisticsClass;

  size_t playedCount;
  double sumOfRewards;
  double sumOfSquaredRewards;
};

typedef ReferenceCountedObjectPtr<BanditStatistics> BanditStatisticsPtr;

class DiscreteBanditPolicy : public Object
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
  friend class DiscreteBanditPolicyClass;

  size_t timeStep;
  std::vector<BanditStatisticsPtr> banditStatistics;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) = 0;
};

typedef ReferenceCountedObjectPtr<DiscreteBanditPolicy> DiscreteBanditPolicyPtr;

class IndexBasedDiscreteBanditPolicy : public DiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const = 0;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    if (timeStep < banditStatistics.size())
      return timeStep; // play each bandit once
    return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

  size_t selectMaximumIndexBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
    for (size_t i = 0; i < banditStatistics.size(); ++i)
    {
      double score = computeBanditScore(i, timeStep, banditStatistics);
      if (score > bestScore)
        bestBandit = i, bestScore = score;
    }
    return bestBandit;
  }
};

class UCB1DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }
};

class UCB1TunedDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    size_t nj = statistics->getPlayedCount();
    double lnn = log((double)timeStep);
    double varianceUB = statistics->getRewardVariance() + sqrt(2 * lnn / nj);
    return statistics->getRewardMean() + sqrt((lnn / nj) * juce::jmin(0.25, varianceUB));
  }
};

// TODO:
// UCB1TunedBanditsPolicy
// UCB1NormalBanditsPolicy
// EpsilonGreedyBanditsPolicy

class UCB2DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  UCB2DiscreteBanditPolicy(double alpha = 1.0) : alpha(alpha) {}
 
  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    episodeCounts.clear();
    episodeCounts.resize(numBandits, 0);
    episodeRemainingSteps = 0;
    currentBandit = 0;
  }

protected:
  friend class UCB2DiscreteBanditPolicyClass;

  double alpha;
  std::vector<size_t> episodeCounts;
  size_t episodeRemainingSteps;
  size_t currentBandit;

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tauEpisodeCount = tau(episodeCounts[banditNumber]);
    double e = 1.0; // FIXME: what is e ????
    return statistics->getRewardMean() + sqrt((1.0 + alpha) * log(e * timeStep / tauEpisodeCount) / (2.0 * tauEpisodeCount));
  }

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    while (episodeRemainingSteps == 0)
    {
      currentBandit = selectMaximumIndexBandit(timeStep, banditStatistics);
      size_t& rj = episodeCounts[currentBandit];
      episodeRemainingSteps = tau(rj + 1) - tau(rj);
      ++rj;
    }

    --episodeRemainingSteps;
    return currentBandit;
  }

  size_t tau(size_t count) const
    {return (size_t)ceil(pow(1 + alpha, (double)count));}
};

class EpsilonGreedyDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  EpsilonGreedyDiscreteBanditPolicy(double c, double d)
    : c(c), d(d), random(new RandomGenerator()) {}
  EpsilonGreedyDiscreteBanditPolicy() : c(0.0), d(0.0) {}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[banditNumber]->getRewardMean();}
 
  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    double epsilon = juce::jmin(1.0, c * numBandits / (d * d * timeStep));
    if (random->sampleBool(epsilon))
      return random->sampleSize(numBandits);
    else
      return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

protected:
  friend class EpsilonGreedyDiscreteBanditPolicyClass;

  double c;
  double d;
  RandomGeneratorPtr random;
};

/*
** Sand Box
*/
class BanditsSandBox : public WorkUnit
{
public:
  BanditsSandBox() : maxTimeStep(100000), numRuns(100) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    std::vector<bool> insertTimeStepIntoCurve(maxTimeStep + 1, false);
    for (double d = 0.0; d <= log10((double)maxTimeStep); d += 0.25)
    {
      size_t index = (size_t)pow(10.0, d);
      jassert(index < insertTimeStepIntoCurve.size());
      insertTimeStepIntoCurve[index] = true;
    }

    size_t numBandits = 2;
    std::vector<double> probs(numBandits);
    probs[0] = 0.6;
    probs[1] = 0.9;

    std::map<size_t, std::map<String, double> > results;

    context.enterScope(T("UCB2"));

    for (size_t run = 0; run < numRuns; ++run)
    {
      context.progressCallback(new ProgressionState(run, numRuns, T("Runs")));

      DiscreteBanditStatePtr state = new BernouilliDiscreteBanditState(probs, run);
      double bestReward, secondBestReward;
      size_t optimalBandit = state->getOptimalBandit(bestReward, secondBestReward);

      DiscreteBanditPolicyPtr policy = new UCB1TunedDiscreteBanditPolicy();
        // new UCB2DiscreteBanditPolicy(0.001);
        // new EpsilonGreedyDiscreteBanditPolicy(0.10, bestReward - secondBestReward);
        //new UCB1TunedDiscreteBanditPolicy();
      policy->initialize(numBandits);
      double sumOfRewards = 0.0;
      size_t numberOfTimesOptimalIsPlayed = 0;
      for (size_t timeStep = 1; timeStep <= maxTimeStep; ++timeStep)
      {
        size_t action = 1;//policy->selectNextBandit();
        double reward;
        state->performTransition(action, reward);
        policy->updatePolicy(action, reward);

        sumOfRewards += reward;
        if (action == optimalBandit)
          ++numberOfTimesOptimalIsPlayed;

        if (insertTimeStepIntoCurve[timeStep])
        {
          std::map<String, double>& res = results[timeStep];

          res[T("bestMachinePlayed")] += 100.0 * numberOfTimesOptimalIsPlayed / (double)timeStep;
          res[T("actualRegret")] += timeStep * bestReward - sumOfRewards;
          res[T("sumOfRewards")] += sumOfRewards;
          res[T("optimalSumOfRewards")] += timeStep * bestReward;
/*
          context.enterScope(T("Time Step ") + String((int)timeStep));
          context.resultCallback(T("log10(timeStep)"), log10((double)timeStep));
          context.resultCallback(T("bestMachinePlayed"), 100.0 * numberOfTimesOptimalIsPlayed / (double)timeStep);
          context.resultCallback(T("actualRegret"), timeStep * bestReward - sumOfRewards);
          context.resultCallback(T("sumOfRewards"), sumOfRewards);
          context.resultCallback(T("optimalSumOfRewards"), timeStep * bestReward);
          context.resultCallback(T("timeStep"), timeStep);
          context.leaveScope(true);*/
        }
      }
    }

    context.leaveScope(true);

    context.enterScope(T("UCB2"));
    for (std::map<size_t, std::map<String, double> >::const_iterator it = results.begin(); it != results.end(); ++it)
    {
      size_t timeStep = it->first;
      context.enterScope(T("timeStep" ) + String((int)timeStep));
      const std::map<String, double>& res = it->second;
      context.resultCallback(T("log10(timeStep)"), log10((double)timeStep));
      for (std::map<String, double>::const_iterator it2 = res.begin(); it2 != res.end(); ++it2)
        context.resultCallback(it2->first, it2->second / numRuns);
      context.leaveScope(true);
    }
    context.leaveScope(true);
    return true;
  }

  bool testRandom(ExecutionContext& context)
  {
    RandomGeneratorPtr random = RandomGenerator::getInstance();

    context.enterScope(T("testRandom"));

    for (size_t i = 0; i < 100; ++i)
    {
      size_t pouet = 0;
      for (size_t j = 0; j < 10; ++j)
        if (random->sampleBool(0.9))
          ++pouet;
      context.enterScope(T("hop"));
        context.resultCallback(T("i"), i);
        context.resultCallback(T("m"), pouet / 10.0);
      context.leaveScope(true);
    }

    context.leaveScope(true);
    return true;
  }

protected:
  friend class BanditsSandBoxClass;

  size_t maxTimeStep;
  size_t numRuns;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
