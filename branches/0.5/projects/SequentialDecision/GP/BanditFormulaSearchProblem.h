/*-----------------------------------------.---------------------------------.
| Filename: BanditFormaulSearchProblem.h   | Evaluate Bandit Formula        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_SEARCH_PROBLEM_BANDIT_FORMULA_H_
# define LBCPP_GP_SEARCH_PROBLEM_BANDIT_FORMULA_H_

# include "FormulaSearchProblem.h"
# include "../Bandits/DiscreteBanditPolicy.h"
# include <algorithm>

namespace lbcpp
{

class BanditProblemSampler : public Sampler
{
public:
  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const = 0;

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return new DiscreteBanditState(sampleArms(random));}
};

typedef ReferenceCountedObjectPtr<BanditProblemSampler> BanditProblemSamplerPtr;

class ConstantBanditProblemSampler : public BanditProblemSampler
{
public:
  ConstantBanditProblemSampler(const std::vector<SamplerPtr>& arms)
    : arms(arms) {}

  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const
    {return arms;}

protected:
  std::vector<SamplerPtr> arms;
};

// Icaart
class Setup0BanditProblemSampler : public BanditProblemSampler
{
public:
  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const
  {
    std::vector<SamplerPtr> arms(2);
    for (size_t i = 0; i < arms.size(); ++i)
      arms[i] = bernoulliSampler(random->sampleDouble(0.0, 1.0));
    return arms;
  }
}; 

// Bernoulli bandits
class Setup1BanditProblemSampler : public BanditProblemSampler
{
public:
  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const
  {
    std::vector<SamplerPtr> arms(random->sampleSize(2, 10));
    for (size_t i = 0; i < arms.size(); ++i)
      arms[i] = bernoulliSampler(random->sampleDouble(0.0, 1.0));
    return arms;
  }
};

// Small-gap Bernoulli bandits
class Setup2BanditProblemSampler : public BanditProblemSampler
{
public:
  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const
  {
    std::vector<SamplerPtr> arms(random->sampleSize(2, 10));
    double highestProbability = random->sampleDouble(0.5, 1.0);
    for (size_t i = 0; i < arms.size(); ++i)
      arms[i] = bernoulliSampler(highestProbability - i * 0.05);
    return arms;
  }
};

// Mixed variance Gaussian bandits
class Setup3BanditProblemSampler : public BanditProblemSampler
{
public:
  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const
  {
    std::vector<SamplerPtr> arms(10);
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double stddev = pow(10.0, (double)random->sampleInt(-2, 1)); // sample in {0.01, 0.1, 1.0}
      arms[i] = gaussianSampler(random->sampleDouble(0.0, 1.0), stddev);
    }
    return arms;
  }
};

// Many arms
class Setup4BanditProblemSampler : public BanditProblemSampler
{
public:
  virtual std::vector<SamplerPtr> sampleArms(const RandomGeneratorPtr& random) const
  {
    std::vector<SamplerPtr> arms(1000);
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double stddev = pow(10.0, (double)random->sampleInt(-2, 1)); // sample in {0.01, 0.1, 1.0}
      arms[i] = gaussianSampler(random->sampleDouble(0.0, 1.0), stddev);
    }
    return arms;
  }
};

class BanditFormulaObjective : public SimpleUnaryFunction
{
public:
  BanditFormulaObjective(bool isSimpleRegret = false, BanditProblemSamplerPtr problemSampler = BanditProblemSamplerPtr(), size_t horizon = 100)
    : SimpleUnaryFunction(sumType(gpExpressionClass, discreteBanditPolicyClass), regretScoreObjectClass),
      isSimpleRegret(isSimpleRegret), problemSampler(problemSampler), horizon(horizon) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DiscreteBanditPolicyPtr policy = input.dynamicCast<DiscreteBanditPolicy>();
    if (!policy)
    {
      GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();
      policy = gpExpressionDiscreteBanditPolicy(formula);
    }
    
    std::vector<SamplerPtr> arms = problemSampler->sampleArms(context.getRandomGenerator());
    std::vector<double> expectedRewards(arms.size());
    double bestRewardExpectation = 0.0;
    double meanRewardExpectation = 0.0;
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double p = arms[i]->computeExpectation().getDouble();
      if (p > bestRewardExpectation) 
        bestRewardExpectation = p;
      meanRewardExpectation += p;
      expectedRewards[i] = p;
    }
    meanRewardExpectation /= arms.size();

    double regret = sampleRegret(context, policy, arms, expectedRewards, bestRewardExpectation);
    //double meanUniformRegret = bestRewardExpectation - meanRewardExpectation;
    return new RegretScoreObject(regret, isSimpleRegret ? 1 : horizon);
  }
 
protected:
  friend class BanditFormulaObjectiveClass;

  bool isSimpleRegret;
  BanditProblemSamplerPtr problemSampler;
  size_t horizon;

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }

  double sampleRegret(ExecutionContext& context, DiscreteBanditPolicyPtr pol, const std::vector<SamplerPtr>& arms, const std::vector<double>& expectedRewards, double bestRewardExpectation) const
  {
    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);    
    DiscreteBanditPolicyPtr policy = pol->cloneAndCast<DiscreteBanditPolicy>();
    policy->initialize(arms.size());
    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }

    if (isSimpleRegret)
    {
      double bestReward = 0.0;
      size_t bestEstimatedArm = 0;
      for (size_t i = 0; i < arms.size(); ++i)
      {
        double reward = policy->getRewardEmpiricalMean(i);
        if (reward > bestReward)
          bestReward = reward, bestEstimatedArm = i;
      }
      return bestRewardExpectation - expectedRewards[bestEstimatedArm]; // simple regret 
    }
    else
      return horizon * bestRewardExpectation - sumOfRewards; // regret
  }
};

typedef ReferenceCountedObjectPtr<BanditFormulaObjective> BanditFormulaObjectivePtr;

class BanditFormulaSearchProblem : public FormulaSearchProblem
{
public:
  BanditFormulaSearchProblem() : numArmsInSampling(5) {}

  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return gpExpressionDiscreteBanditPolicyVariablesEnumeration;}

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpOpposite; i <= gpAbs; ++i)
      if (i != gpExp)
        unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpMax; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    size_t index = 0;

    for (; index < count; ++index)
    {
      double t = juce::jmax(1, (int)pow(10.0, random->sampleDouble(-0.1, 5)));
      jassert(numArmsInSampling >= 2);
      std::vector<double> input(4 * numArmsInSampling);
      size_t i = 0;
      /*input[i++] = 0.0;
      input[i++] = 0.0;
      input[i++] = 1.0;
      input[i++] = t;
      */
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        input[i++] = juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1)); // rk1
        input[i++] = juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1)); // sk1
        input[i++] = juce::jlimit(1, (int)t, (int)(t * random->sampleDouble(-0.1, 1.1))); // tk1
        input[i++] = t;
      }
      res[index] = input;
    }
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
      {return (fabs(left.second - right.second) > 1e-12 ? left.second < right.second : left.first < right.first);}
  };

  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    BinaryKeyPtr res = new BinaryKey(inputSamples.size() * numArmsInSampling);
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      const std::vector<double>& variables = inputSamples[i];
      
      std::vector< std::pair<size_t, double> > values(numArmsInSampling);
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        double value = expression->compute(&variables[j * 4]);
        if (!isNumberValid(value))
          return BinaryKeyPtr();
        values[j] = std::make_pair(j, value);
      }
      std::sort(values.begin(), values.end(), ValueComparator());
  
      jassert(numArmsInSampling <= 128);
      for (size_t j = 0; j < numArmsInSampling; ++j)
      {
        bool isHigherThanPrevious = (j > 0 && values[j].second > values[j-1].second);
        res->pushByte((unsigned char)(values[j].first + (isHigherThanPrevious ? 128 : 0)));
      }
    }
    return res;
  }

protected:
  friend class BanditFormulaSearchProblemClass;

  BanditFormulaObjectivePtr objective;
  size_t numArmsInSampling;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GP_SEARCH_PROBLEM_BANDIT_FORMULA_H_
