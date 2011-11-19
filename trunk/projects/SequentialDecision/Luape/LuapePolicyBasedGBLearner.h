/*-----------------------------------------.---------------------------------.
| Filename: LuapePolicyBasedGBLearner.h    | Luape Policy Based Gradient     |
| Author  : Francis Maes                   |  Boosting Learner               |
| Started : 19/11/2011 16:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_POLICY_BASED_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_POLICY_BASED_GRADIENT_BOOSTING_H_

# include "LuapeGraphLearner.h"
# include "../Core/Policy.h"

namespace lbcpp
{

class LuapeRewardStorage : public Object
{
public:
  LuapeRewardStorage(size_t maximumDeltaTime)
    : maximumDeltaTime(maximumDeltaTime) {}

  void observe(const String& name, double reward, size_t time)
  {
    SparseRewardSequenceMap::iterator it = m.find(name);
    if (it == m.end())
    {
      SparseRewardSequence sequence;
      observe(sequence, reward, time);
      m[name] = sequence;
    }
    else
      observe(it->second, reward, time);
  }

  typedef std::list<std::pair<size_t, double> > SparseRewardSequence;

  SparseRewardSequence getRecentRewards(const String& name) const
  {
    SparseRewardSequenceMap::const_iterator it = m.find(name);
    return it == m.end() ? SparseRewardSequence() : it->second;
  }

private:
  typedef std::map<String, SparseRewardSequence> SparseRewardSequenceMap;

  size_t maximumDeltaTime;
  SparseRewardSequenceMap m;

  void observe(SparseRewardSequence& sequence, double reward, size_t time)
  {
    sequence.push_back(std::make_pair(time, reward));
    if (time > maximumDeltaTime)
    {
      while (sequence.front().first < time - maximumDeltaTime)
        sequence.pop_front();
    }
  }
};

  /*
class Policy : public Object
{
public:
  virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions) = 0;
  virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions) = 0;
  virtual void policyEnd(ExecutionContext& context, double reward, const Variable& finalState) {}

  lbcpp_UseDebuggingNewOperator
};
*/

class LuapePolicyBasedGBLearner : public LuapeGradientBoostingLearner
{
public:
  LuapePolicyBasedGBLearner(const PolicyPtr& policy, double learningRate, size_t maxDepth, size_t budget)
    : LuapeGradientBoostingLearner(learningRate, maxDepth), policy(policy), budget(budget) {}

  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context, const DenseDoubleVectorPtr& predictions) const
  {
    context.enterScope(T("Computing optimal weak learner"));
    double optimalReward = -DBL_MAX;
    LuapeNodePtr optimalWeakLearner;
    findOptimalWeakLearner(context, new LuapeGraphBuilderState(problem, graph->cloneAndCast<LuapeGraph>(), maxDepth), optimalReward, optimalWeakLearner);
    context.leaveScope(optimalReward);

    context.enterScope(T("Weak Learning"));
    double bestReward = -DBL_MAX;
    LuapeNodePtr bestWeakLearner;
    for (size_t i = 0; i < budget; ++i)
    {
      double reward;
      LuapeYieldNodePtr yieldNode = sampleTrajectory(context, reward);
      if (yieldNode && reward > bestReward)
        bestWeakLearner = yieldNode->getArgument(), bestReward = reward;
      context.progressCallback(new ProgressionState(i+1, budget, T("Trajectories")));
    }
    context.leaveScope(bestReward);
    context.informationCallback(T("Weak learner: ") + bestWeakLearner->toShortString() + T(" [") + String(bestReward) + T("]"));
    context.informationCallback(T("Optimal weak learner: ") + optimalWeakLearner->toShortString() + T(" [") + String(optimalReward) + T("]"));
    context.resultCallback(T("regret"), optimalReward - bestReward);
    const_cast<LuapePolicyBasedGBLearner* >(this)->regret.push(optimalReward - bestReward);
    context.resultCallback(T("averageRegret"), regret.getMean());

    context.informationCallback(T("Average Regret: ") + String(regret.getMean()));
    return bestWeakLearner;
  }

  LuapeYieldNodePtr sampleTrajectory(ExecutionContext& context, double& reward) const
  {
    LuapeGraphBuilderStatePtr builder = new LuapeGraphBuilderState(problem, graph->cloneAndCast<LuapeGraph>(), maxDepth);

    bool isFirstStep = true;
    bool noMoreActions = false;
    while (!builder->isFinalState())
    {
      ContainerPtr actions = builder->getAvailableActions();
      if (!actions->getNumElements())
      {
        noMoreActions = true;
        break;
      }

      Variable action;
      if (isFirstStep)
      {
        action = policy->policyStart(context, builder, builder->getAvailableActions());
        isFirstStep = false;
      }
      else
        action = policy->policyStep(context, 0.0, builder, builder->getAvailableActions());
      double reward;
      builder->performTransition(context, action, reward);
    }

    LuapeYieldNodePtr yieldNode = builder->getGraph()->getLastNode().dynamicCast<LuapeYieldNode>();
    reward = yieldNode ? computeCompletionReward(context, yieldNode->getArgument()) : 0.0;

    policy->policyEnd(context, reward, builder);
    if (noMoreActions)
      context.informationCallback(T("Out-of-actions: ") + builder->toShortString());
    else
      context.informationCallback(T("Final State: ") + builder->toShortString() + T(" => ") + String(reward));
    return builder->getGraph()->getLastNode().dynamicCast<LuapeYieldNode>();
  }

  void findOptimalWeakLearner(ExecutionContext& context, const LuapeGraphBuilderStatePtr& state, double& bestReward, LuapeNodePtr& bestWeakLearner) const
  {
    if (state->isFinalState())
    {
      LuapeYieldNodePtr yieldNode = state->getGraph()->getLastNode().dynamicCast<LuapeYieldNode>();
      if (yieldNode)
      {
        LuapeNodePtr node = yieldNode->getArgument();
        double reward = computeCompletionReward(context, node);
        if (reward > bestReward)
          bestReward = reward, bestWeakLearner = node;
      }
    }
    else
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable stateBackup;
        double reward;
        state->performTransition(context, actions->getElement(i), reward, &stateBackup);
        //context.enterScope(state->toShortString());
        findOptimalWeakLearner(context, state, bestReward, bestWeakLearner);
        //context.leaveScope();
        state->undoTransition(context, stateBackup);
      }
    }
  }
protected:
  size_t budget;
  PolicyPtr policy;
  ScalarVariableStatistics regret;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_POLICY_BASED_GRADIENT_BOOSTING_H_
