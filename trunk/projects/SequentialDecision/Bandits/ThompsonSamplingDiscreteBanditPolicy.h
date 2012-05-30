/*-----------------------------------------.---------------------------------.
| Filename: ThompsonSamplingDiscreteBan...h| Thompson Sampling for           |
| Author  : Francis Maes                   |  Discrete MAB problems          |
| Started : 30/05/2012 12:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_THOMPSON_SAMPLING_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_THOMPSON_SAMPLING_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{

class ThompsonSamplingDiscreteBanditPolicy : public TwoParametersIndexBasedDiscreteBanditPolicy
{
public:
  ThompsonSamplingDiscreteBanditPolicy(double alpha = 1.0, double beta = 1.0)
    : TwoParametersIndexBasedDiscreteBanditPolicy(alpha, beta) {}

  virtual void getParameterRanges(double& alphaMin, double& alphaMax, double& betaMin, double& betaMax) const
    {alphaMin = betaMin = 0.0; alphaMax = 2.0; betaMax = 4.0;}

  virtual double computeBanditScore(size_t armNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[armNumber]->getRewardMean();}

  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    size_t numArms = banditStatistics.size();
    std::vector<size_t> bestArms;
    double bestScore = -DBL_MAX;

    //double alpha = pow(10.0, this->alpha);
    //double beta = pow(10.0, this->beta);

    for (size_t i = 0; i < numArms; ++i)
    {
      BanditStatisticsPtr stats = banditStatistics[i];
      
      double a = stats->getRewardSum() + alpha;
      double b = ((double)stats->getPlayedCount() - stats->getRewardSum()) + beta;
      double score = a == 0 ? 1.0 : (b == 0 ? 0.0 : random->sampleFromBeta(a, b));
      if (score >= bestScore)
      {
        if (score > bestScore)
        {
          bestArms.clear();
          bestScore = score;
        }
        bestArms.push_back(i);
      }
    }
    return bestArms.size() ? bestArms[random->sampleSize(bestArms.size())] : random->sampleSize(numArms);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_THOMPSON_SAMPLING_H_
