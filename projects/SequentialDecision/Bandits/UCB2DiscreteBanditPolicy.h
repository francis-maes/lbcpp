/*-----------------------------------------.---------------------------------.
| Filename: UCB2DiscreteBanditPolicy.h     | UCB2 policy                     |
| Author  : Francis Maes                   |                                 |
| Started : 24/05/2011 20:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_UCB2_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_UCB2_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{

class UCB2DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  UCB2DiscreteBanditPolicy(double alpha = 1.0) : alpha(alpha) {}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(-3, 2);}

  virtual void setParameters(const Variable& parameters)
    {alpha = pow(10, parameters.getDouble());}

  virtual Variable getParameters() const
    {return log10(alpha);}

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    episodeCounts.clear();
    episodeCounts.resize(numBandits, 0);
    tauValues.reserve(10000);
    episodeRemainingSteps = 0;
    currentBandit = 0;
  }

protected:
  friend class UCB2DiscreteBanditPolicyClass;

  double alpha;
  std::vector<size_t> episodeCounts;
  std::vector<size_t> tauValues;
  size_t episodeRemainingSteps;
  size_t currentBandit;

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tauEpisodeCount = tau(episodeCounts[banditNumber]);
    double e = 2.71828183;
    return statistics->getRewardMean() + sqrt((1.0 + alpha) * log(e * timeStep / tauEpisodeCount) / (2.0 * tauEpisodeCount));
  }

  static size_t argmax(const std::vector<double>& values)
  {
    size_t res = (size_t)-1;
    double m = -DBL_MAX;
    for (size_t i = 0; i < values.size(); ++i)
    {
      double v = values[i];
      if (v > m)
        m = v, res = i;
    }
    return res;
  }


  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    if (episodeRemainingSteps == 0)
    {
      std::vector<double> banditScores(numBandits);
      for (size_t i = 0; i < banditScores.size(); ++i)
        banditScores[i] = computeBanditScore(i, timeStep, banditStatistics);

      while (true)
      {
        currentBandit = argmax(banditScores);
        size_t& rj = episodeCounts[currentBandit];
        episodeRemainingSteps = tau(rj + 1) - tau(rj);
        ++rj;

        if (episodeRemainingSteps == 0)
          banditScores[currentBandit] = computeBanditScore(currentBandit, timeStep, banditStatistics);
        else
          break;
      }
    }

    --episodeRemainingSteps;
    return currentBandit;
  }

  size_t tau(size_t count) const
  {
    if (count >= tauValues.size())
    {
      std::vector<size_t>& v = const_cast<UCB2DiscreteBanditPolicy* >(this)->tauValues;
      v.resize(count + 1);
      v[count] = (size_t)ceil(pow(1 + alpha, (double)count));
    }
    return tauValues[count];
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_UCB2_H_
