/*-----------------------------------------.---------------------------------.
| Filename: LuapeBanditFormulaSearchProblem.h | Bandit Formula Discovery     |
| Author  : Francis Maes                   |                                 |
| Started : 02/05/2012 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FORMULA_DISCOVERY_BANDIT_H_
# define LBCPP_LUAPE_FORMULA_DISCOVERY_BANDIT_H_

# include "LuapeFormulaDiscovery.h"

namespace lbcpp
{

class LuapeBanditFormulaSearchProblem : public ExpressionSearchProblem
{
public:
  LuapeBanditFormulaSearchProblem(BanditProblemSamplerPtr problemSampler, size_t horizon)
    : problemSampler(problemSampler), horizon(horizon)
  {
  }
  LuapeBanditFormulaSearchProblem() : horizon(0) {}

  virtual bool initializeProblem(ExecutionContext& context)
  {
    addConstant(1.0);
    addConstant(2.0);
    addConstant(3.0);
    addConstant(5.0);
    addConstant(7.0);

    addInput(doubleType, "rk");
    addInput(doubleType, "sk");
    addInput(doubleType, "tk");
    addInput(doubleType, "t");

    addFunction(oppositeDoubleLuapeFunction());
    addFunction(inverseDoubleLuapeFunction());
    addFunction(sqrtDoubleLuapeFunction());
    addFunction(logDoubleLuapeFunction());
    addFunction(absDoubleLuapeFunction());

    addFunction(addDoubleLuapeFunction());
    addFunction(subDoubleLuapeFunction());
    addFunction(mulDoubleLuapeFunction());
    addFunction(divDoubleLuapeFunction());
    addFunction(minDoubleLuapeFunction());
    addFunction(maxDoubleLuapeFunction());

    addTargetType(doubleType);

    samplesCache = sampleInputs(context.getRandomGenerator());
    return true;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = (double)horizon; best = 0.0;}

  virtual double computeObjective(ExecutionContext& context, const ExpressionPtr& node, size_t instanceIndex)
  {
    const std::vector<SamplerPtr>& arms = getProblem(context, instanceIndex);
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

    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);    
    DiscreteBanditPolicyPtr policy = new Policy(refCountedPointerFromThis(this), node);
    policy->initialize(arms.size());
    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = performBanditStep(context, state, policy);
      sumOfRewards += expectedRewards[action];
    }
    return horizon * bestRewardExpectation - sumOfRewards;
  }

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const ExpressionPtr& node) const
  {
    LuapeSampleVectorPtr samples = samplesCache->getSamples(context, node);
    if (!samples)
      return BinaryKeyPtr();

    LuapeSampleVector::const_iterator it = samples->begin();
    BinaryKeyPtr res = new BinaryKey(numBanditSamples * numArmsPerBanditSample);
    for (size_t i = 0; i < numBanditSamples; ++i)
    {
      std::vector<std::pair<size_t, double> > values(numArmsPerBanditSample);
      for (size_t j = 0; j < numArmsPerBanditSample; ++j)
      {
        double value = it.getRawDouble(); ++it;
        if (value == doubleMissingValue)
          return BinaryKeyPtr();
        values[j] = std::make_pair(j, value);
      }
      std::sort(values.begin(), values.end(), ValueComparator());
  
      jassert(numArmsPerBanditSample <= 128);
      for (size_t j = 0; j < numArmsPerBanditSample; ++j)
      {
        bool isHigherThanPrevious = (j > 0 && values[j].second > values[j-1].second);
        res->pushByte((unsigned char)(values[j].first + (isHigherThanPrevious ? 128 : 0)));
      }
    }
    return res;
  }

protected:
  friend class LuapeBanditFormulaSearchProblemClass;

  BanditProblemSamplerPtr problemSampler;
  size_t horizon;

private:
  std::vector< std::vector<SamplerPtr> > problems;
  LuapeSamplesCachePtr samplesCache;

  enum {numBanditSamples = 100, numArmsPerBanditSample = 5};

  LuapeSamplesCachePtr sampleInputs(RandomGeneratorPtr random) const
  {
    LuapeSamplesCachePtr res = createCache(numBanditSamples * numArmsPerBanditSample, 100); // 100 Mb cache

    for (size_t i = 0; i < numBanditSamples; ++i)
    {
      double t = juce::jmax(1, (int)pow(10.0, random->sampleDouble(-0.1, 5)));
      for (size_t j = 0; j < numArmsPerBanditSample; ++j)
      {
        DenseDoubleVectorPtr input = new DenseDoubleVector(4, 0.0);
        input->setValue(0, juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1))); // rk1
        input->setValue(1, juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1))); // sk1
        input->setValue(2, juce::jlimit(1, (int)t, (int)(t * random->sampleDouble(-0.1, 1.1)))); // tk1
        input->setValue(3, (double)t);
        res->setInputObject(inputs, i * numArmsPerBanditSample + j, input);
      }
    }
    return res;
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
      {return (fabs(left.second - right.second) > 1e-12 ? left.second < right.second : left.first < right.first);}
  };

  const std::vector<SamplerPtr>& getProblem(ExecutionContext& context, size_t index)
  {
    while (index >= problems.size())
      problems.push_back(problemSampler->sampleArms(context.getRandomGenerator()));
    return problems[index];
  }

  static size_t performBanditStep(ExecutionContext& context, DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit(context);
    double reward;
    state->performTransition(context, action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }

  struct Policy : public IndexBasedDiscreteBanditPolicy
  {
    Policy(LuapeInferencePtr problem, ExpressionPtr formula)
      : problem(problem), formula(formula) {}

    LuapeInferencePtr problem;
    ExpressionPtr formula;

    virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {
      BanditStatisticsPtr arm = banditStatistics[banditNumber];
      Variable input[4];
      input[0] = arm->getRewardMean();
      input[1] = arm->getRewardStandardDeviation();
      input[2] = (double)arm->getPlayedCount();
      input[3] = (double)timeStep;
      return formula->compute(defaultExecutionContext(), input).toDouble();
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FORMULA_DISCOVERY_BANDIT_H_
