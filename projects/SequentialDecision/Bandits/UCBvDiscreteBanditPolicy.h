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
  
class UCBVDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  UCBVDiscreteBanditPolicy(double c = 1.0, double zeta = 1.0)
    : c(c), zeta(zeta) {}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];

    double mean = bandit->getRewardMean();
    double variance = bandit->getRewardVariance();
    double s = (double)bandit->getPlayedCount();
    double epsilon = zeta * log((double)timeStep);
    return mean + sqrt((2.0 * variance * epsilon) / s) + c * (3.0 * epsilon) / s; 
  }

  virtual SamplerPtr createParametersSampler() const
    {return objectCompositeSampler(pairClass(doubleType, doubleType), gaussianSampler(1.0), gaussianSampler(1.0));}

  virtual void setParameters(const Variable& parameters)
  {
    const PairPtr& pair = parameters.getObjectAndCast<Pair>();
    c = pair->getFirst().getDouble();
    zeta = pair->getSecond().getDouble();
  }

  virtual Variable getParameters() const
    {return new Pair(c, zeta);}

protected:
  friend class UCBVDiscreteBanditPolicyClass;

  double c;
  double zeta;
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_UCBV_H_
