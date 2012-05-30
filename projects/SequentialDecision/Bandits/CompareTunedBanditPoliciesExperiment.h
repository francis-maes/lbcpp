/*-----------------------------------------.---------------------------------.
| Filename: CompareTunedBanditPoliciesE...h| Compare Tuned Bandit Policies   |
| Author  : Francis Maes                   |                                 |
| Started : 29/05/2012 12:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_COMPARE_TUNED_POLICIES_EXPERIMENT_H_
# define LBCPP_BANDITS_COMPARE_TUNED_POLICIES_EXPERIMENT_H_

# include "DiscreteBanditExperiment.h"
# include "FindBanditsFormula.h"
# include <lbcpp/Optimizer/BanditPool.h>

namespace lbcpp
{

class CompareTunedBanditPoliciesExperiment : public WorkUnit
{
public:
  CompareTunedBanditPoliciesExperiment() : numTrainingProblems(10000), numTestingProblems(10000), tuningPrecision(10), numArms(10) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    size_t paramMin = numArms * 2;
    size_t paramMax = numArms * 1000;
    size_t paramStep = numArms;

    std::vector<DiscreteBanditPolicyPtr> noParamPolicies;
    noParamPolicies.push_back(ucb1DiscreteBanditPolicy());
    noParamPolicies.push_back(ucb1TunedDiscreteBanditPolicy());
    noParamPolicies.push_back(greedyDiscreteBanditPolicy());
    noParamPolicies.push_back(uniformDiscreteBanditPolicy());

    std::vector<OneParameterIndexBasedDiscreteBanditPolicyPtr> oneParamPolicies;
    oneParamPolicies.push_back(ucb1DiscreteBanditPolicy());
    oneParamPolicies.push_back(klucbDiscreteBanditPolicy());
    oneParamPolicies.push_back(new Formula2CustomIndexBasedDiscreteBanditPolicy()); // pow(tk, C) * rk
/*    oneParamPolicies.push_back(new Formula2IndexBasedDiscreteBanditPolicy()); // tk * (rk - C)
    oneParamPolicies.push_back(new Formula6IndexBasedDiscreteBanditPolicy()); // tk * (rk - C * sk)
    oneParamPolicies.push_back(new Formula7IndexBasedDiscreteBanditPolicy()); // tk * (tk^2 - C * sk)*/

    std::vector<TwoParametersIndexBasedDiscreteBanditPolicyPtr> twoParamPolicies;
    twoParamPolicies.push_back(overExploitDiscreteBanditPolicy());
    twoParamPolicies.push_back(exploreExploitDiscreteBanditPolicy());
    twoParamPolicies.push_back(ucbvDiscreteBanditPolicy());
    twoParamPolicies.push_back(epsilonGreedyDiscreteBanditPolicy());
    twoParamPolicies.push_back(thompsonSamplingDiscreteBanditPolicy());

    context.enterScope(T("Curve"));
    for (size_t p = paramMin; p <= paramMax; p += paramStep)
    {
      context.enterScope(T("Param = ") + String((int)p));
      context.resultCallback(T("problem_param"), p);
      //context.resultCallback(T("log(problem_param)"), log10((double)p));

      context.enterScope(T("Sampling training and testing problems"));
      size_t horizon;
      SamplerPtr problemSampler = createProblemSampler(p, horizon);
      context.resultCallback(T("horizon"), horizon);

      RandomGeneratorPtr random = new RandomGenerator(51);
      std::vector<DiscreteBanditStatePtr> trainingProblems(numTrainingProblems), testingProblems(numTestingProblems);
      for (size_t i = 0; i < trainingProblems.size(); ++i)
        trainingProblems[i] = problemSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();
      for (size_t i = 0; i < testingProblems.size(); ++i)
        testingProblems[i] = problemSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();
      context.leaveScope();

      for (size_t i = 0; i < noParamPolicies.size(); ++i)
      {
        DiscreteBanditPolicyPtr policy = noParamPolicies[i];
        evaluatePolicy(context, policy, policy->getClass()->getShortName(), testingProblems, horizon);
      }

      for (size_t i = 0; i < oneParamPolicies.size(); ++i)
      {
        OneParameterIndexBasedDiscreteBanditPolicyPtr policy = oneParamPolicies[i];
        String str = policy->getClass()->getShortName();
        context.enterScope(T("Tuning ") + str);
        tuneOneParameterPolicy(context, policy, trainingProblems, horizon);
        context.leaveScope();
        context.resultCallback(str + "_param1", policy->getC());
        evaluatePolicy(context, policy, str, testingProblems, horizon);
      }

      for (size_t i = 0; i < twoParamPolicies.size(); ++i)
      {
        TwoParametersIndexBasedDiscreteBanditPolicyPtr policy = twoParamPolicies[i];
        String str = policy->getClass()->getShortName();
        context.enterScope(T("Tuning ") + str);
        tuneTwoParametersPolicy(context, policy, trainingProblems, horizon);
        context.leaveScope();
        context.resultCallback(str + "_param1", policy->getAlpha());
        context.resultCallback(str + "_param2", policy->getBeta());
        evaluatePolicy(context, policy, str, testingProblems, horizon);
      }

      context.leaveScope();
    }
    context.leaveScope();

    return Variable();
  }

protected:
  friend class CompareTunedBanditPoliciesExperimentClass;

  size_t numTrainingProblems;
  size_t numTestingProblems;
  size_t tuningPrecision;
  size_t numArms;

  SamplerPtr createProblemSampler(size_t hyperParameter, size_t& horizon) const
  {
    horizon = hyperParameter;

    ClassPtr bernoulliSamplerClass = lbcpp::getType(T("BernoulliSampler"));
    SamplerPtr banditSamplerSampler = objectCompositeSampler(bernoulliSamplerClass, uniformScalarSampler(0.0, 1.0));
    return new DiscreteBanditInitialStateSampler(banditSamplerSampler, numArms);
  }

  static double sampleRegret(ExecutionContext& context, DiscreteBanditPolicyPtr policy, DiscreteBanditStatePtr problem, size_t horizon)
  {
    std::vector<SamplerPtr> arms = problem->getArms();
    std::vector<double> expectedRewards(arms.size());
    double bestRewardExpectation = 0.0;
    for (size_t i = 0; i < arms.size(); ++i)
    {
      double p = arms[i]->computeExpectation().getDouble();
      if (p > bestRewardExpectation) 
        bestRewardExpectation = p;
      expectedRewards[i] = p;
    }

    DiscreteBanditStatePtr state = new DiscreteBanditState(arms);
    policy->initialize(arms.size());
    double sumOfRewards = 0.0;
    for (size_t timeStep = 1; timeStep <= horizon; ++timeStep)
    {
      size_t action = policy->selectNextBandit(context);
      double reward;
      state->performTransition(context, action, reward);
      policy->updatePolicy(action, reward);
      sumOfRewards += expectedRewards[action];
    }
    return horizon * bestRewardExpectation - sumOfRewards; // regret
  }

  struct Objective : public BanditPoolObjective
  {
    Objective(DiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& problems, size_t horizon)
      : policy(policy), problems(problems), horizon(horizon) {}

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = (double)horizon; best = 0.0;}

    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
      {return sampleRegret(context, Parameterized::cloneWithNewParameters(this->policy, parameter), problems[instanceIndex % problems.size()], horizon);}

  private:
    DiscreteBanditPolicyPtr policy;
    const std::vector<DiscreteBanditStatePtr>& problems;
    size_t horizon;
  };
  
  void tuneOneParameterPolicy(ExecutionContext& context, OneParameterIndexBasedDiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& problems, size_t horizon)
  {
    BanditPoolObjectivePtr objective(new Objective(policy, problems, horizon));
    BanditPoolPtr pool = new BanditPool(objective, 10.0, false, false); // TODO: try multi-threading here

    pool->reserveArms(tuningPrecision);
    double minValue, maxValue;
    policy->getParameterRange(minValue, maxValue);
    TypePtr type = Parameterized::getParametersType(policy);
    size_t precision = tuningPrecision * tuningPrecision;
    for (size_t i = 0; i <= precision; ++i)
      pool->createArm(minValue + (maxValue - minValue) * i / (double)precision);
    
    tunePolicy(context, policy, problems, pool);
  }

  void tuneTwoParametersPolicy(ExecutionContext& context, TwoParametersIndexBasedDiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& problems, size_t horizon)
  {
    BanditPoolObjectivePtr objective(new Objective(policy, problems, horizon));
    BanditPoolPtr pool = new BanditPool(objective, 5.0, false, false); // TODO: try multi-threading here

    pool->reserveArms(tuningPrecision * tuningPrecision);
    double alphaMin, alphaMax, betaMin, betaMax;
    policy->getParameterRanges(alphaMin, alphaMax, betaMin, betaMax);
    TypePtr type = Parameterized::getParametersType(policy);
    for (size_t i = 0; i <= tuningPrecision; ++i)
      for (size_t j = 0; j <= tuningPrecision; ++j)
      {
        double alpha = alphaMin + (alphaMax - alphaMin) * i / (double)tuningPrecision;
        double beta = betaMin + (betaMax - betaMin) * j / (double)tuningPrecision;
        pool->createArm(new Pair(type, alpha, beta));
      }

    tunePolicy(context, policy, problems, pool);
  }

  void tunePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& problems, BanditPoolPtr pool)
  {
    pool->playIterations(context, tuningPrecision, problems.size());
    size_t armIndex = pool->sampleArmWithHighestReward(context);
    Variable bestParameter = pool->getArmParameter(armIndex);
    dynamic_cast<Parameterized* >(policy.get())->setParameters(bestParameter);
  }

  double evaluatePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, const String& shortName, const std::vector<DiscreteBanditStatePtr>& problems, size_t horizon)
  {
    size_t numEstimationsPerProblem = 1;

    context.enterScope(T("Evaluating ") + policy->toShortString());
    ScalarVariableMean mean;
    for (size_t i = 0; i < problems.size(); ++i)
    {
      DiscreteBanditStatePtr problem = problems[i];
      for (size_t j = 0; j < numEstimationsPerProblem; ++j)
        mean.push(sampleRegret(context, policy->cloneAndCast<DiscreteBanditPolicy>(), problem, horizon));
    }
    double regret = mean.getMean();
    context.leaveScope(regret);
    context.resultCallback(shortName + "_regret", regret);
    context.resultCallback(shortName + "_avgregret", regret / (double)horizon);
    return regret;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_COMPARE_TUNED_POLICIES_EXPERIMENT_H_
