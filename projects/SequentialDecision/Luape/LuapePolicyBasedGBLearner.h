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
# include <list>

namespace lbcpp
{

class LuapeRewardStorage : public Object
{
public:
  LuapeRewardStorage(EnumerationPtr features, size_t memorySize)
    : features(features), memorySize(memorySize) {}

  void startNewEpisode()
  {
    observations.push_back(std::vector<ScalarVariableMeanAndVariance>(features->getNumElements()));
    if (observations.size() > memorySize)
      observations.pop_front();
  }

  void observe(const DenseDoubleVectorPtr& counts, double reward)
  {
    jassert(observations.size());
    std::vector<ScalarVariableMeanAndVariance>& currentObservations = observations.back();
    size_t n = counts->getNumValues();
    if (currentObservations.size() < n)
      currentObservations.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      double weight = counts->getValue(i);
      if (weight)
        currentObservations[i].push(reward, weight);
    }
  }

  double getExpectedReward(size_t featureIndex, double discount, double& sampleCount) const
  {
    double res = 0.0;
    sampleCount = 0.0;
    double weight = 1.0;
    ObservationList::const_reverse_iterator it = observations.rbegin(); // skip first element
    for (++it; it != observations.rend(); ++it)
      if (featureIndex < it->size())
      {
        const ScalarVariableMeanAndVariance& stats = (*it)[featureIndex];
        double count = stats.getCount();
        double reward = stats.getMean();
        res += reward * count * weight;
        sampleCount += count * weight;
        weight *= discount;
      }
    if (sampleCount)
      res /= sampleCount;
    return res;
  }

protected:
  EnumerationPtr features;
  size_t memorySize;

  typedef std::list< std::vector<ScalarVariableMeanAndVariance> > ObservationList;

  ObservationList observations;
};

typedef ReferenceCountedObjectPtr<LuapeRewardStorage> LuapeRewardStoragePtr;

class LuapeRewardStorageBasedPolicy : public Policy
{
public:
  LuapeRewardStorageBasedPolicy()
  {
    features = new DefaultEnumeration();
    rewards = new LuapeRewardStorage(features, 1000);
  }

  /////// FEATURES //////////
  void addFeature(ExecutionContext& context, const String& name, const SparseDoubleVectorPtr& res) const
  {
    size_t index = features.staticCast<DefaultEnumeration>()->findOrAddElement(context, name);
    res->setElement(index, 1.0);
  }

  void addPredicateConjunctionsToFeatures(ExecutionContext& context, const std::vector<String>& predicates, const SparseDoubleVectorPtr& res,
                                            size_t predicateIndex, std::vector<bool>& included, size_t includedCount, String& name) const
  {
    if (predicateIndex == predicates.size())
    {
      addFeature(context, name, res);
    }
    else
    {
      // subsets without this predicate
      included[predicateIndex] = false;
      addPredicateConjunctionsToFeatures(context, predicates, res, predicateIndex + 1, included, includedCount, name);
      
      // subsets with this predicate
      if (includedCount < 3)
      {
        included[predicateIndex] = true;
        String previousName = name;
        name = (name.isEmpty() ? "" : name + T(" ")) + predicates[predicateIndex];
        addPredicateConjunctionsToFeatures(context, predicates, res, predicateIndex + 1, included, includedCount + 1, name);
        name = previousName;
      }
    }
  }

  SparseDoubleVectorPtr makeFeatures(ExecutionContext& context, const DecisionProblemStatePtr& s, const Variable& a) const
  {
    const LuapeGraphBuilderStatePtr& state = s.staticCast<LuapeGraphBuilderState>();
    const LuapeGraphBuilderActionPtr& action = a.getObjectAndCast<LuapeGraphBuilderAction>();

    SparseDoubleVectorPtr res(new SparseDoubleVector(features, doubleType));

    std::vector<String> predicates;
    predicates.push_back("step=" + String(state->getCurrentStep()));
    predicates.push_back("stacksize=" + String(state->getStackSize()));

    LuapeNodePtr nodeToAdd = action->getNodeToAdd();
    if (action->isNewNode())
    {
      if (nodeToAdd.isInstanceOf<LuapeYieldNode>())
      {
        predicates.push_back("kind=yield");
      }
      else
      {
        LuapeFunctionNodePtr functionNode = nodeToAdd.staticCast<LuapeFunctionNode>();

        predicates.push_back("kind=apply");
        predicates.push_back("fun=" + functionNode->getFunction()->getClassName());
        for (size_t i = 0; i < functionNode->getNumArguments(); ++i)
          predicates.push_back("arg" + String((int)i) + "type=" + functionNode->getArgument(i)->getType()->getName());
      }
    }
    else
    {
      jassert(nodeToAdd);

      predicates.push_back("kind=push");
      predicates.push_back("argtype=" + nodeToAdd->getType()->getName());
      if (state->getStackSize() > 0)
        predicates.push_back("prevargtype=" + state->getStackElement(state->getStackSize() - 1)->getType()->getName());
      else
        predicates.push_back("firstarg");
    }

    std::vector<bool> included(predicates.size(), false);
    String name;
    size_t includedCount = 0;
    addPredicateConjunctionsToFeatures(context, predicates, res, 0, included, includedCount, name);

    return res;
  }

  void beginEpisode()
  {
    rewards->startNewEpisode();
  }

  /////// POLICY //////////
  virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions)
  {
    trajectoryFeatures = new DenseDoubleVector(features, doubleType);
    return selectAction(context, state.getObjectAndCast<DecisionProblemState>(), actions);
  }

  virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions)
    {return selectAction(context, state.getObjectAndCast<DecisionProblemState>(), actions);}

  virtual void policyEnd(ExecutionContext& context, double reward, const Variable& finalState)
    {rewards->observe(trajectoryFeatures, reward);}

  Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state, const ContainerPtr& actions)
  {
    Variable action = sampleAction(context, state, actions);
    SparseDoubleVectorPtr features = makeFeatures(context, state, action);
    features->addTo(trajectoryFeatures);
    return action;
  }

  Variable sampleAction(ExecutionContext& context, const DecisionProblemStatePtr& state, const ContainerPtr& actions)
  {
    size_t n = actions->getNumElements();
    std::vector<size_t> bestActions;
    double bestScore = -DBL_MAX;
    for (size_t i = 0; i < n; ++i)
    {
      double score = computeActionScore(context, state, actions->getElement(i));
      if (score >= bestScore)
      {
        if (score > bestScore)
        {
          bestScore = score;
          bestActions.clear();
        }
        bestActions.push_back(i);
      }
    }
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (bestActions.size())
      return actions->getElement(bestActions[random->sampleSize(bestActions.size())]);
    else
      return actions->getElement(random->sampleSize(n));
  }

  double computeActionScore(ExecutionContext& context, const DecisionProblemStatePtr& state, const Variable& action)
  {
    SparseDoubleVectorPtr features = makeFeatures(context, state, action);
    double sampleCount = 0.0;
    double sumOfRewards = 0.0;
    double sumOfWeights = 0.0;

    for (size_t i = 0; i < features->getNumValues(); ++i)
    {
      const std::pair<size_t, double>& feature = features->getValue(i);
      double samples;
      double reward = rewards->getExpectedReward(feature.first, 1.0, samples);
      sampleCount += feature.second * samples;
      sumOfRewards += feature.second * reward;
      sumOfWeights += feature.second;
    }
    sampleCount /= sumOfWeights;
    sumOfRewards /= sumOfWeights;

    return sumOfRewards + 1.5 / (1.0 + sampleCount);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  EnumerationPtr features;
  DenseDoubleVectorPtr trajectoryFeatures;
  LuapeRewardStoragePtr rewards;
};

class LuapePolicyBasedGBLearner : public LuapeGradientBoostingLearner
{
public:
  LuapePolicyBasedGBLearner(const PolicyPtr& policy, double learningRate, size_t maxDepth, size_t budget)
    : LuapeGradientBoostingLearner(learningRate, maxDepth), policy(policy), budget(budget)
  {
  }

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
  {
    if (!LuapeGradientBoostingLearner::initialize(context, problem, function))
      return false;
    typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(problem, maxDepth);
    typeSearchSpace->pruneStates(context);
    return true;
  }


  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context, const DenseDoubleVectorPtr& predictions) const
  {
    static const bool computeOptimalLearner = true;

    double optimalReward = -DBL_MAX;
    LuapeNodePtr optimalWeakLearner;
    if (computeOptimalLearner)
    {
      context.enterScope(T("Computing optimal weak learner"));
      findOptimalWeakLearner(context, new LuapeGraphBuilderState(graph->cloneAndCast<LuapeGraph>(), typeSearchSpace), optimalReward, optimalWeakLearner);
      context.leaveScope(optimalReward);
    }

    // FIXME !
    if (policy.dynamicCast<LuapeRewardStorageBasedPolicy>())
      policy.staticCast<LuapeRewardStorageBasedPolicy>()->beginEpisode();
    // -

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
    if (bestWeakLearner)
      context.informationCallback(T("Weak learner: ") + bestWeakLearner->toShortString() + T(" [") + String(bestReward) + T("]"));
    else
      context.errorCallback(T("Failed to find a weak learner"));

    if (computeOptimalLearner)
    {
      context.informationCallback(T("Optimal weak learner: ") + optimalWeakLearner->toShortString() + T(" [") + String(optimalReward) + T("]"));
      context.resultCallback(T("regret"), optimalReward - bestReward);
      const_cast<LuapePolicyBasedGBLearner* >(this)->regret.push(optimalReward - bestReward);
      context.resultCallback(T("averageRegret"), regret.getMean());
      context.informationCallback(T("Average Regret: ") + String(regret.getMean()));
    }
    return bestWeakLearner;
  }

  LuapeYieldNodePtr sampleTrajectory(ExecutionContext& context, double& reward) const
  {
    LuapeGraphBuilderStatePtr builder = new LuapeGraphBuilderState(graph->cloneAndCast<LuapeGraph>(), typeSearchSpace);

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
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_POLICY_BASED_GRADIENT_BOOSTING_H_
