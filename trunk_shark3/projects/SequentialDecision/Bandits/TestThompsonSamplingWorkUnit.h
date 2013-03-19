/*-----------------------------------------.---------------------------------.
| Filename: TestThompsonSamplingWorkUnit.h | Test Thompson Sampling          |
| Author  : Francis Maes                   |                                 |
| Started : 30/05/2012 15:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_TEST_THOMPSON_SAMPLING_WORK_UNIT_H_
# define LBCPP_BANDITS_TEST_THOMPSON_SAMPLING_WORK_UNIT_H_

# include "DiscreteBanditExperiment.h"

namespace lbcpp
{

class TestThompsonSamplingWorkUnit : public WorkUnit
{
public:
  TestThompsonSamplingWorkUnit() : K(10), epsilon(0.1) {}

  void testBetaSampling(ExecutionContext& context, double alpha, double beta)
  {
    context.enterScope(T("Beta(") + String(alpha) + T(", ") + String(beta) + T(")"));

    std::vector<size_t> bins(100, 0);
    static const size_t count = 10000;
    for (size_t i = 0; i < count; ++i)
    {
      double sample = context.getRandomGenerator()->sampleFromBeta(alpha, beta);
      jassert(sample >= 0.0 && sample <= 1.0);
      size_t index = (size_t)(sample * bins.size());
      if (index >= bins.size())
        index = bins.size() - 1;
      bins[index]++;
    }
    for (size_t i = 0; i < bins.size(); ++i)
    {
      context.enterScope(T("Bin ") + String((int)i));
      context.resultCallback("x", i / 100.0 + 0.5);
      context.resultCallback("count", bins[i]);
      context.leaveScope();
    }
    context.leaveScope();
  }

  virtual Variable run(ExecutionContext& context)
  {
   /* testBetaSampling(context, 0.5, 0.5);
    testBetaSampling(context, 5.0, 1.0);
    testBetaSampling(context, 1.0, 3.0);
    testBetaSampling(context, 2.0, 2.0);
    testBetaSampling(context, 2.0, 5.0);*/

    for (size_t iteration = 0; iteration < 10; ++iteration)
    {
      context.enterScope(T("Iteration ") + String((int)iteration + 1));

      std::vector<SamplerPtr> arms(K);
      arms[0] = bernoulliSampler(0.5);
      for (size_t i = 1; i < K; ++i)
        arms[i] = bernoulliSampler(0.5 - epsilon);
      DiscreteBanditStatePtr problem = new DiscreteBanditState(arms);

      DiscreteBanditPolicyPtr policy = thompsonSamplingDiscreteBanditPolicy(1.0, 1.0);

      policy->initialize(K);
      double sumOfRewards = 0.0;

      size_t timeStep = 1;
      size_t nextTimeStep = 10;
      double lastRegret = 0.0;

      while (nextTimeStep <= 1000000)
      {
        context.enterScope(String((int)nextTimeStep));
        context.resultCallback("t", nextTimeStep);
        context.resultCallback("log(t)", log10((double)nextTimeStep));
        while (timeStep < nextTimeStep)
        {
          size_t action = policy->selectNextBandit(context);
          double reward;
          problem->performTransition(context, action, reward);
          policy->updatePolicy(action, reward);
          sumOfRewards += 0.5 - (action ? epsilon : 0.0);
          ++timeStep;
        }
        double regret = timeStep * 0.5 - sumOfRewards;
        lastRegret = regret;
        context.leaveScope(regret);
        nextTimeStep *= 10;
      }
      context.leaveScope(lastRegret);
    }
    return true;
  }

protected:
  friend class TestThompsonSamplingWorkUnitClass;

  size_t K;
  double epsilon;
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_TEST_THOMPSON_SAMPLING_WORK_UNIT_H_
