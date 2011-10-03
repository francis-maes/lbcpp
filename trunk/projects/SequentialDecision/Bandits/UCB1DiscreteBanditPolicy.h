/*-----------------------------------------.---------------------------------.
| Filename: UCB1DiscreteBanditPolicy.h     | UCB2 policy                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/05/2011 20:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_UCB1_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_UCB1_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{

// FIXME: ranger
class GreedyDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[banditNumber]->getRewardMean();}
};

class UniformDiscreteBanditPolicy : public DiscreteBanditPolicy
{
public:
  virtual void initialize(size_t numBandits)
    {nextArm = 0; DiscreteBanditPolicy::initialize(numBandits);}
 
protected:
  size_t nextArm;

  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t res = nextArm;
    nextArm = (nextArm + 1) % banditStatistics.size();
    return res;
  }
};
// --

class UCB1DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  UCB1DiscreteBanditPolicy(double C = 2.0)
    : C(C) {}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(2.0, 1.0);}

  virtual void setParameters(const Variable& parameters)
    {C = parameters.toDouble();}

  virtual Variable getParameters() const
    {return C;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + sqrt(C * log((double)timeStep) / statistics->getPlayedCount());
  }

protected:
  friend class UCB1DiscreteBanditPolicyClass;

  double C;
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

class UCB1NormalDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double qj = statistics->getSquaredRewardSum();
    double nj = (double)statistics->getPlayedCount();
    double xj2 = statistics->getRewardMean() * statistics->getRewardMean();
    double n = (double)timeStep;
    return statistics->getRewardMean() + sqrt(16.0 * ((qj - nj * xj2) / (nj - 1)) * (log(n - 1.0) / nj));
  }

  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    if (timeStep < banditStatistics.size())
      return timeStep; // play each bandit once

    // if there is a machine which has been played less then 8log(n) times, then play this machine
    size_t limit = (size_t)ceil(8 * log((double)timeStep));
    for (size_t i = 0; i < banditStatistics.size(); ++i)
      if (banditStatistics[i]->getPlayedCount() < limit)
        return i;

    return selectMaximumIndexBandit(context, timeStep, banditStatistics);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_UCB1_H_
