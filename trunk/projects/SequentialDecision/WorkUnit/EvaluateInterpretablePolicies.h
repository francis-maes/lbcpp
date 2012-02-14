/*-----------------------------------------.---------------------------------.
| Filename: EvaluateInterpretablePolicies.h| Evaluate Interpretable Policies |
| Author  : Francis Maes                   |                                 |
| Started : 14/02/2012 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_EVALUATE_INTERPRETABLE_POLICIES_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_EVALUATE_INTERPRETABLE_POLICIES_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/DecisionProblem/DecisionProblem.h>
# include "../GP/PolicyFormulaSearchProblem.h"

namespace lbcpp
{

class GreedyPolicy : public Policy
{
public:
  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    jassert(n != 0);

    std::vector<Variable> bestActions;
    double bestReward = -DBL_MAX;
    for (size_t i = 0; i < n; ++i)
    {
      DecisionProblemStatePtr s = state->cloneAndCast<DecisionProblemState>();
      Variable a = actions->getElement(i);
      double reward;
      s->performTransition(context, a, reward);
      if (reward >= bestReward)
      {
        if (reward > bestReward)
          bestActions.clear();
        bestActions.push_back(a);
      }
    }
    
    RandomGeneratorPtr random = context.getRandomGenerator();
    return bestActions.size() ? bestActions[random->sampleSize(bestActions.size())] : actions->getElement(random->sampleSize(n));
  }

  lbcpp_UseDebuggingNewOperator
};

class EvaluateInterpretablePolicies : public WorkUnit
{
public:
  EvaluateInterpretablePolicies() : numTrajectories(100000)
  {
  }

  virtual Variable run(ExecutionContext& context)
  {
    std::vector<DecisionProblemPtr> problems;
    getProblems(problems);

    for (size_t i = 0; i < problems.size(); ++i)
    {
      DecisionProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      evaluatePoliciesOnProblem(context, problem);
      context.leaveScope();
    }
    return true;
  }

  void evaluatePoliciesOnProblem(ExecutionContext& context, DecisionProblemPtr problem)
  {
    evaluatePolicyOnProblem(context, problem, randomPolicy(), "random");
    evaluatePolicyOnProblem(context, problem, new GreedyPolicy(), "greedy");
  }

  struct EvaluatePolicyFromInitialState : public WorkUnit
  {
    EvaluatePolicyFromInitialState(DecisionProblemPtr problem, DecisionProblemStatePtr initialState, PolicyPtr policy, size_t numTrajectories)
      : problem(problem), initialState(initialState), policy(policy->cloneAndCast<Policy>()), numTrajectories(numTrajectories) {}

    DecisionProblemPtr problem;
    DecisionProblemStatePtr initialState;
    PolicyPtr policy;
    size_t numTrajectories;

    virtual Variable run(ExecutionContext& context)
    {
      size_t T = problem->getHorizon();
      double discount = problem->getDiscount();
        
      policy->startEpisodes(context);
      double res = 0.0;
      for (size_t i = 0; i < numTrajectories; ++i)
      {
        DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();
        policy->startEpisode(context, problem, state);
        double cumulativeReward = 0.0;
        double weight = 1.0;
        for (size_t t = 0; t < T && !state->isFinalState(); ++t)
        {
          Variable action = policy->selectAction(context, state);
          double reward;
          state->performTransition(context, action, reward);
          cumulativeReward += reward * weight;
          weight *= discount;
        }
        res += cumulativeReward;
      }
      res /= numTrajectories;
      return res;
    }
  };

  double evaluatePolicyOnProblem(ExecutionContext& context, DecisionProblemPtr problem, PolicyPtr policy, const String& policyName)
  {
    size_t dummy;
    ObjectVectorPtr initialStates = problem->getValidationInitialStates(dummy);
    size_t n = initialStates->getNumElements();

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(policyName, n);
    for (size_t i = 0; i < n; ++i)
      workUnit->setWorkUnit(i, new EvaluatePolicyFromInitialState(problem, initialStates->getAndCast<DecisionProblemState>(i), policy, numTrajectories));

    context.enterScope(workUnit->toShortString());
    ContainerPtr results = context.run(workUnit, false).getObjectAndCast<Container>();

    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += results->getElement(i).getDouble();
    res /= (double)n;
    context.leaveScope(res);
    return res;
  }

protected:
  friend class EvaluateInterpretablePoliciesClass;

  size_t numTrajectories;

  void getProblems(std::vector<DecisionProblemPtr>& res)
  {
    res.push_back(DecisionProblem::create(getType("LinearPointPhysicProblem")));
    res.push_back(DecisionProblem::create(getType("LeftOrRightControlProblem")));
    res.push_back(DecisionProblem::create(getType("BicyleBalancingProblem")));
    DecisionProblemPtr pb = DecisionProblem::create(getType("BicyleBalancingProblem"));
    pb->setVariable(3, true);
    res.push_back(pb);
    res.push_back(DecisionProblem::create(getType("HIVDecisionProblem")));
    res.push_back(DecisionProblem::create(getType("CarOnTheHillProblem")));
    res.push_back(DecisionProblem::create(getType("AcrobotProblem")));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
