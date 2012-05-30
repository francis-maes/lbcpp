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
    {alphaMin = betaMin = 0.0; alphaMax = betaMax = 5.0;}

  virtual double computeBanditScore(size_t armNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[armNumber]->getRewardMean();}
 
  static double sampleFromBeta(RandomGeneratorPtr random, double alpha, double beta, double lambda = 0.0)
  {
    double gamma = 0;
    jassert(alpha > 0 && beta > 0);
    if (alpha < 1)
    {
      double b = 1 + alpha * exp(-1.0);
      while (true)
      {
        double p = b * random->sampleDouble();
        if (p>1)
        {
          gamma = -log((b-p)/alpha);
          if (random->sampleDouble() <= pow(gamma, alpha-1))
            break;
        }
        else
        {
          gamma = pow(p,1/alpha);
          if (random->sampleDouble() <= exp(-gamma))
            break;
        }
      }
    }
    else if (alpha == 1)
      gamma = -log(random->sampleDouble());
    else
    {
      double y = -log(random->sampleDouble());
      while (random->sampleDouble() > pow(y * exp(1 - y), alpha - 1))
        y = -log(random->sampleDouble());
      gamma = alpha * y;
    }
    return beta * gamma + lambda;
  }

  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    size_t numArms = banditStatistics.size();
    std::vector<size_t> bestArms;
    double bestScore = -DBL_MAX;

    double alpha = pow(10.0, this->alpha);
    double beta = pow(10.0, this->beta);

    for (size_t i = 0; i < numArms; ++i)
    {
      BanditStatisticsPtr stats = banditStatistics[i];
      
      double a = stats->getRewardSum() + alpha;
      double b = ((double)stats->getPlayedCount() - stats->getRewardSum()) + beta;
      double score = a == 0 ? 1.0 : (b == 0 ? 0.0 : sampleFromBeta(random, a, b));
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
