/*-----------------------------------------.---------------------------------.
| Filename: UCBVDiscreteBanditPolicy.h     | UCB-v policy                    |
| Author  : Francis Maes                   |                                 |
| Started : 24/05/2011 20:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_UCBV_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_UCBV_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{
  
  // alpha = c; beta = zeta
class UCBVDiscreteBanditPolicy : public TwoParametersIndexBasedDiscreteBanditPolicy
{
public:
  UCBVDiscreteBanditPolicy(double c = 1.0, double zeta = 1.0)
    : TwoParametersIndexBasedDiscreteBanditPolicy(c, zeta) {}

  virtual void getParameterRanges(double& cMin, double& cMax, double& zetaMin, double& zetaMax) const
  {
    cMin = zetaMin = -1.0;
    cMax = zetaMax = 1.0;
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];

    double mean = bandit->getRewardMean();
    double variance = bandit->getRewardVariance();
    double s = (double)bandit->getPlayedCount();
    double epsilon = beta * log((double)timeStep);
    return mean + sqrt((2.0 * variance * epsilon) / s) + alpha * (3.0 * epsilon) / s; 
  }
};

// TODO: ranger...
class OverExploitDiscreteBanditPolicy : public TwoParametersIndexBasedDiscreteBanditPolicy
{
public:
  OverExploitDiscreteBanditPolicy(double alpha = 0.5, double beta = 0.0) : TwoParametersIndexBasedDiscreteBanditPolicy(alpha, beta) {}

  virtual void getParameterRanges(double& alphaMin, double& alphaMax, double& betaMin, double& betaMax) const
  {
    alphaMin = betaMin = 0.0;
    alphaMax = betaMax = 1.0;
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];

    double rk = bandit->getRewardMean();
    double tk = (double)bandit->getPlayedCount();
    return pow(tk, alpha) * (rk - beta);
  }
};

class ExploreExploitDiscreteBanditPolicy : public TwoParametersIndexBasedDiscreteBanditPolicy
{
public:
  ExploreExploitDiscreteBanditPolicy(double alpha = 0.5, double beta = 1.0) : TwoParametersIndexBasedDiscreteBanditPolicy(alpha, beta) {}

  virtual void getParameterRanges(double& alphaMin, double& alphaMax, double& betaMin, double& betaMax) const
  {
    alphaMin = betaMin = 0.0;
    alphaMax = 1.0;
    betaMax = 2.0;
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];

    double rk = bandit->getRewardMean();
    double tk = (double)bandit->getPlayedCount();
    return rk + beta / pow(tk, alpha);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_UCBV_H_
