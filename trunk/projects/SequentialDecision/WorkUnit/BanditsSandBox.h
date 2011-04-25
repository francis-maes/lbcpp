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
    double e = 1.0;//2.71828183;
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

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    random->setSeed(16645186);
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[banditNumber]->getRewardMean();}
 
  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    double epsilon = juce::jmin(1.0, c * numBandits / (d * d * (timeStep + 1)));
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
class EvaluateDiscreteBanditPolicyWorkUnit : public WorkUnit
{
public:
  EvaluateDiscreteBanditPolicyWorkUnit(size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates, const DiscreteBanditPolicyPtr& policy, bool verbose = true)
    : numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), policy(policy), verbose(verbose) {}

  EvaluateDiscreteBanditPolicyWorkUnit()
    : numBandits(0), maxTimeStep(0), verbose(false) {}

  virtual String toShortString() const
    {return T("Evaluate ") + policy->getClass()->getShortName() + T("(") +  policy->toShortString() + T(") on ") + String((int)initialStates.size()) + T(" problems");}

  virtual Variable run(ExecutionContext& context)
  {
    // compute timeSteps
    std::vector<size_t> timeSteps;
    size_t timeStep = 0;
    size_t batchSize = 2;//numBandits;
    size_t batchIndex = 0;
    while (timeStep < maxTimeStep)
    {
      timeStep += batchSize;
      timeSteps.push_back(timeStep);
      ++batchIndex;
      batchSize *= 2;//numBandits;
    }
    
    // initial accumulators
    std::vector<double> actualRegretVector(timeSteps.size(), 0.0);
    std::vector<double> bestMachinePlayedVector(timeSteps.size(), 0.0);

    ScalarVariableStatisticsPtr actualRegretStatistics = new ScalarVariableStatistics(T("actualRegret")); 

    // main calculation loop
    for (size_t i = 0; i < initialStates.size(); ++i)
    {
      DiscreteBanditStatePtr state = initialStates[i]->cloneAndCast<DiscreteBanditState>();
      double bestReward, secondBestReward;
      size_t optimalBandit = state->getOptimalBandit(bestReward, secondBestReward);
      policy->initialize(numBandits);

      double sumOfRewards = 0.0;
      size_t numberOfTimesOptimalIsPlayed = 0;
      timeStep = 0;
      for (size_t j = 0; j < timeSteps.size(); ++j)
      {
        size_t numTimeSteps = timeSteps[j] - (j > 0 ? timeSteps[j - 1] : 0);
        for (size_t k = 0; k < numTimeSteps; ++k, ++timeStep)
           performBanditStep(state, policy, optimalBandit, sumOfRewards, numberOfTimesOptimalIsPlayed);
        jassert(timeStep == timeSteps[j]);
 
        actualRegretVector[j] += timeStep * bestReward - sumOfRewards;
        if (timeStep <= numBandits)
          bestMachinePlayedVector[j] += 1.0 / (double)numBandits;
        else
          bestMachinePlayedVector[j] += numberOfTimesOptimalIsPlayed / (double)timeStep;
      }
      actualRegretStatistics->push(timeStep * bestReward - sumOfRewards);

      if (verbose)
        context.progressCallback(new ProgressionState(i + 1, initialStates.size(), T("Problems")));
    }

    double invZ = 1.0 / initialStates.size();
    for (size_t i = 0; i < timeSteps.size(); ++i)
    {
      actualRegretVector[i] *= invZ;
      bestMachinePlayedVector[i] *= invZ;
      if (verbose)
      {
        size_t timeStep = timeSteps[i];
        context.enterScope(String((int)timeStep) + T(" Steps"));
        context.resultCallback(T("log10(n)"), log10((double)timeStep));
        context.resultCallback(T("actualRegret"), actualRegretVector[i]);
        context.resultCallback(T("bestMachinePlayed"), bestMachinePlayedVector[i] * 100.0);
        context.resultCallback(T("n"), timeStep);
        context.leaveScope(actualRegretVector[i]);
      }
    }

    return actualRegretStatistics;
  }
 
protected:
  friend class EvaluateDiscreteBanditPolicyWorkUnitClass;

  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  DiscreteBanditPolicyPtr policy;
  bool verbose;

  void performBanditStep(DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy, size_t optimalBandit, double& sumOfRewards, size_t& numberOfTimesOptimalIsPlayed)
  {
    size_t action = policy->selectNextBandit();
    double reward;
    state->performTransition(action, reward);
    policy->updatePolicy(action, reward);
    sumOfRewards += state->getExpectedReward(action);
    if (action == optimalBandit)
      ++numberOfTimesOptimalIsPlayed;
  }
};

class BanditsSandBox : public WorkUnit
{
public:
  BanditsSandBox() : numBandits(2), maxTimeStep(100000), numTrainingProblems(100), numTestingProblems(100) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    jassert(numBandits == 10);
    std::vector<double> probs(numBandits);
    probs[0] = 0.9;
    probs[1] = 0.8;
    probs[2] = 0.8;
    probs[3] = 0.8;
    probs[4] = 0.8;
    probs[5] = 0.8;
    probs[6] = 0.8;
    probs[7] = 0.8;
    probs[8] = 0.8;
    probs[9] = 0.8;

    RandomGeneratorPtr random = RandomGenerator::getInstance();

    std::vector<DiscreteBanditStatePtr> trainingStates(numTrainingProblems);
    for (size_t i = 0; i < trainingStates.size(); ++i)
      trainingStates[i] = new BernouilliDiscreteBanditState(probs, random->sampleUint32());

    std::vector<DiscreteBanditStatePtr> testingStates(numTestingProblems);
    for (size_t i = 0; i < testingStates.size(); ++i)
      testingStates[i] = new BernouilliDiscreteBanditState(probs, random->sampleUint32());

    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(new UCB2DiscreteBanditPolicy(0.001));
    policies.push_back(new UCB1TunedDiscreteBanditPolicy());
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.05, probs[0] - probs[1]));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.10, probs[0] - probs[1]));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.15, probs[0] - probs[1]));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.20, probs[0] - probs[1]));
    policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.25, probs[0] - probs[1]));

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies "), policies.size());
    for (size_t i = 0; i < policies.size(); ++i)
      workUnit->setWorkUnit(i, new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, testingStates, policies[i], true));

    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(true);

    context.run(workUnit);
    return true;
  }


protected:
  friend class BanditsSandBoxClass;

  size_t numBandits;
  size_t maxTimeStep;

  size_t numTrainingProblems;
  size_t numTestingProblems;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
