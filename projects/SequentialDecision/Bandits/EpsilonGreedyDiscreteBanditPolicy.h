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

class EpsilonGreedyDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  EpsilonGreedyDiscreteBanditPolicy(double c, double d)
    : c(c), d(d), random(new RandomGenerator()) {}
  EpsilonGreedyDiscreteBanditPolicy() : c(0.0), d(0.0), random(new RandomGenerator()) {}

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    random->setSeed(16645186);
  }

  virtual SamplerPtr createParametersSampler() const
    {return objectCompositeSampler(pairClass(doubleType, doubleType), gaussianSampler(0.5, 0.5), gaussianSampler(0.5, 0.5));}

  virtual void setParameters(const Variable& parameters)
  {
    const PairPtr& pair = parameters.getObjectAndCast<Pair>();
    c = pair->getFirst().getDouble();
    d = pair->getSecond().getDouble();
  }

  virtual Variable getParameters() const
    {return new Pair(c, d);}

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

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    IndexBasedDiscreteBanditPolicy::clone(context, target);
    target.staticCast<EpsilonGreedyDiscreteBanditPolicy>()->random = random->cloneAndCast<RandomGenerator>();
  }

protected:
  friend class EpsilonGreedyDiscreteBanditPolicyClass;

  double c;
  double d;
  RandomGeneratorPtr random;
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_EPSILON_GREEDY_H_
