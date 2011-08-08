/*-----------------------------------------.---------------------------------.
| Filename: KLUCBDiscreteBanditPolicy.h    | KL-UCB policy                   |
| Author  : Francis Maes                   |                                 |
| Started : 05/08/2011 15:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_KL_UCB_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_KL_UCB_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{

class KLUCBDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  KLUCBDiscreteBanditPolicy(double c = 0.0)
    : c(c) {}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(0.0, 3.0);}

  virtual void setParameters(const Variable& parameters)
    {c = parameters.toDouble();}

  virtual Variable getParameters() const
    {return c;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];

    double logT = log((double)timeStep);
    double limit = (logT + c * log(logT)) / statistics->getPlayedCount();
    double rk = statistics->getRewardMean();

    double min = rk;
    double max = 1.0;
    static const double epsilon = 1e-4;
    while (fabs(max - min) > epsilon)
    {
      double middle = (min + max) / 2;
      if (kl(rk, middle) > limit)
        max = middle;
      else
        min = middle;
    }
    return min;
  }

protected:
  friend class KLUCBDiscreteBanditPolicyClass;

  double c;

  static double aLogAOverB(double a, double b)
  {
    if (!a)
      return 0.0;
    else if (!b)
      return DBL_MAX;
    else
      return a * log(a / b);
  }

  static double kl(double p, double q)
    {return aLogAOverB(p, q) + aLogAOverB(1 - p, 1 - q);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_KL_UCB_H_
