/*-----------------------------------------.---------------------------------.
| Filename: DiscreteBanditPolicy.h         | Discrete Bandit Policy base     |
| Author  : Francis Maes                   |       classes                   |
| Started : 24/05/2011 20:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_H_

# include "DiscreteBanditDecisionProblem.h"
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class Parameterized
{
public:
  virtual ~Parameterized() {}

  virtual SamplerPtr createParametersSampler() const = 0;

  virtual void setParameters(const Variable& parameters) = 0;
  virtual Variable getParameters() const = 0;
  virtual TypePtr getParametersType() const
    {return getParameters().getType();}

  static Parameterized* get(const ObjectPtr& object)
    {return dynamic_cast<Parameterized* >(object.get());}

  static TypePtr getParametersType(const ObjectPtr& object)
    {Parameterized* p = get(object); return p ? p->getParametersType() : TypePtr();}

  static ObjectPtr cloneWithNewParameters(const ObjectPtr& object, const Variable& newParameters)
  {
    ObjectPtr res = object->clone(defaultExecutionContext());
    Parameterized* parameterized = get(res);
    jassert(parameterized);
    parameterized->setParameters(newParameters);
    return res;
  }
};

class GPExpression;
typedef ReferenceCountedObjectPtr<GPExpression> GPExpressionPtr;

extern ClassPtr banditStatisticsClass;

class BanditStatistics : public Object
{
public:
  BanditStatistics()
    : Object(banditStatisticsClass), statistics(new ScalarVariableStatistics(T("reward")))
  {
  }

  void update(double reward)
    {statistics->push(reward);}

  size_t getPlayedCount() const
    {return (size_t)statistics->getCount();}

  double getRewardMean() const
    {return statistics->getMean();}

  double getRewardSum()const
    {return statistics->getSum();}

  double getSquaredRewardMean() const
    {return statistics->getSquaresMean();}

  double getSquaredRewardSum() const
    {return statistics->getSquaresMean() * getPlayedCount();}

  double getRewardVariance() const
    {return statistics->getVariance();}

  double getRewardStandardDeviation() const
    {return statistics->getStandardDeviation();}

  double getMinReward() const
    {return statistics->getMinimum();}

  double getMaxReward() const
    {return statistics->getMaximum();}

private:
  friend class BanditStatisticsClass;

  ScalarVariableStatisticsPtr statistics;
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

  size_t selectNextBandit(ExecutionContext& context)
    {return selectBandit(context, timeStep, banditStatistics);}

  virtual void updatePolicy(size_t banditNumber, double reward)
  {
    jassert(banditNumber < banditStatistics.size());
    ++timeStep;
    banditStatistics[banditNumber]->update(reward);
  }

  double getRewardEmpiricalMean(size_t banditNumber) const
    {return banditStatistics[banditNumber]->getRewardMean();}
  size_t getBanditPlayedCount(size_t banditNumber) const
    {return banditStatistics[banditNumber]->getPlayedCount();}

protected:
  friend class DiscreteBanditPolicyClass;

  size_t timeStep;
  std::vector<BanditStatisticsPtr> banditStatistics;

  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) = 0;
};

typedef ReferenceCountedObjectPtr<DiscreteBanditPolicy> DiscreteBanditPolicyPtr;

class IndexBasedDiscreteBanditPolicy : public DiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const = 0;

  virtual size_t selectBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    if (timeStep < banditStatistics.size())
      return timeStep; // play each bandit once
    return selectMaximumIndexBandit(context, timeStep, banditStatistics);
  }

  size_t selectMaximumIndexBandit(ExecutionContext& context, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    double bestScore = -DBL_MAX;
    size_t numBests = 0;
    size_t bestBandit = 0;
    size_t numBandits = banditStatistics.size();
    std::vector<double> scores(numBandits);
    for (size_t i = 0; i < numBandits; ++i)
    {
      double score = computeBanditScore(i, timeStep, banditStatistics);
      scores[i] = score;
      if (score == bestScore)
        ++numBests;
      else if (score > bestScore)
        bestScore = score, numBests = 1, bestBandit = i;
    }
    const RandomGeneratorPtr& random = context.getRandomGenerator();
    if (numBests == 0 || bestScore == -DBL_MAX)
      return random->sampleSize(numBandits);
    else if (numBests == 1)
      return bestBandit;
    else
    {
      size_t index = random->sampleSize(numBests);
      for (size_t i = 0; i < numBandits; ++i)
        if (scores[i] == bestScore)
        {
          if (!index)
            return i;
          --index;
        }
    }
    jassert(false);
    return 0;
  }
};

typedef ReferenceCountedObjectPtr<IndexBasedDiscreteBanditPolicy> IndexBasedDiscreteBanditPolicyPtr;

extern DiscreteBanditPolicyPtr uniformDiscreteBanditPolicy();

extern IndexBasedDiscreteBanditPolicyPtr greedyDiscreteBanditPolicy();

extern IndexBasedDiscreteBanditPolicyPtr ucb1DiscreteBanditPolicy(double C = 2.0);
extern IndexBasedDiscreteBanditPolicyPtr ucb1TunedDiscreteBanditPolicy();
extern IndexBasedDiscreteBanditPolicyPtr ucb1NormalDiscreteBanditPolicy();
extern IndexBasedDiscreteBanditPolicyPtr ucb2DiscreteBanditPolicy(double alpha = 0.001);
extern IndexBasedDiscreteBanditPolicyPtr ucbvDiscreteBanditPolicy(double c = 1.0, double zeta = 1.0);
extern IndexBasedDiscreteBanditPolicyPtr epsilonGreedyDiscreteBanditPolicy(double c = 1.0, double d = 1.0);
extern IndexBasedDiscreteBanditPolicyPtr klucbDiscreteBanditPolicy(double c = 0.0);

extern IndexBasedDiscreteBanditPolicyPtr powerDiscreteBanditPolicy(size_t maxPower, bool useSparseSampler);

extern EnumerationPtr gpExpressionDiscreteBanditPolicyVariablesEnumeration;
extern IndexBasedDiscreteBanditPolicyPtr gpExpressionDiscreteBanditPolicy(GPExpressionPtr expression = GPExpressionPtr());

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_H_
