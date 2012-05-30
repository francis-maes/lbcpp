/*-----------------------------------------.---------------------------------.
| Filename: EpsilonGreedyDiscreteBanditPolicy.h | epsilon greedy Policy      |
| Author  : Francis Maes                   |                                 |
| Started : 24/05/2011 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_EPSILON_GREEDY_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_EPSILON_GREEDY_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{

// alpha = c, beta = d
class EpsilonGreedyDiscreteBanditPolicy : public TwoParametersIndexBasedDiscreteBanditPolicy
{
public:
  EpsilonGreedyDiscreteBanditPolicy(double c = 0.0, double d = 0.0)
    : TwoParametersIndexBasedDiscreteBanditPolicy(c, d) {}

  virtual void getParameterRanges(double& alphaMin, double& alphaMax, double& betaMin, double& betaMax) const
  {
    alphaMin = 0.0; alphaMax = 5.0;
    betaMin = 0.0; betaMax = 1.0;
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[banditNumber]->getRewardMean();}
 
  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numArms = banditStatistics.size();
    if (timeStep < numArms)
      return timeStep; // play each bandit once

    double epsilon = beta ? juce::jmin(1.0, alpha * numArms / (beta * beta * (timeStep + 1))) : 1.0;
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    if (random->sampleBool(epsilon))
      return random->sampleSize(numArms);
    else
      return selectMaximumIndexBandit(context, timeStep, banditStatistics);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_EPSILON_GREEDY_H_
