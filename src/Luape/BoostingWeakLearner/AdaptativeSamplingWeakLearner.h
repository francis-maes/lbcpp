/*-----------------------------------------.---------------------------------.
| Filename: AdaptativeSamplingWeakLearner.h| Adaptative Sampling Weak Learner|
| Author  : Francis Maes                   |                                 |
| Started : 20/12/2011 16:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_ADAPTATIVE_SAMPLING_WEAK_H_
# define LBCPP_LUAPE_LEARNER_ADAPTATIVE_SAMPLING_WEAK_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/LuapeCache.h>
# include "LuapeGraphBuilder.h"
# include <algorithm>

namespace lbcpp
{

class AdaptativeSequentialBuilderWeakLearner : public SequentialBuilderWeakLearner
{
public:
  AdaptativeSequentialBuilderWeakLearner(size_t numWeakNodes, size_t maxSteps, bool useVariableRelevancies, bool useExtendedVariables)
    : SequentialBuilderWeakLearner(numWeakNodes, maxSteps), useVariableRelevancies(useVariableRelevancies), useExtendedVariables(useExtendedVariables) {}
  AdaptativeSequentialBuilderWeakLearner() : useVariableRelevancies(false), useExtendedVariables(false) {}

  typedef std::vector<ObjectPtr> Trajectory;

  virtual void observeStateActionReward(ExecutionContext& context, size_t stepNumber, const std::vector<LuapeNodePtr>& stack, const ObjectPtr& object, double weakObjective, double weight) = 0;
  
  virtual void observeBestWeakNode(ExecutionContext& context,  const LuapeLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, double weakObjective)
  {
    if (weakObjective == -DBL_MAX)
      return;
    jassert(isNumberValid(weakObjective));

    double weight = examples->size() / (double)structureLearner->getTrainingCache()->getNumSamples();
    weakObjective /= weight;

    // normalize objective
    /*context.informationCallback(T("Edge: ") + String(weakObjective) + T(", Weight: ") + String(weight) + T(" Stats: ") + objectiveStats->toShortString() + T(" Node = ") + weakNode->toShortString());
    objectiveStats->push(weakObjective); // FIXME: weight is not taken into account anymore (with ScalarVariableRecentMean)
    if (objectiveStats->getStandardDeviation() == 0.0)
      return;*/

    LuapeNodePtr node = weakNode;
    if (node.isInstanceOf<LuapeConstantNode>())
      return; // those node cannot be produced by this policy
    if (node.isInstanceOf<LuapeFunctionNode>() && node.staticCast<LuapeFunctionNode>()->getFunction()->getClassName() == T("StumpLuapeFunction"))
      node = node.staticCast<LuapeFunctionNode>()->getArgument(0);

    std::vector<Trajectory> trajectories;
    getAllTrajectoriesToBuild(node, maxSteps - 1, trajectories);
    jassert(trajectories.size());
    context.informationCallback(String((int)trajectories.size()) + T(" trajectories to build ") + node->toShortString() + T(" in ") + String((int)maxSteps - 1) + T(" steps"));

    for (size_t i = 0; i < trajectories.size(); ++i)
    {
      //context.informationCallback("Start of trajectory " + String((int)i));
      std::vector<LuapeNodePtr> stack;
      const Trajectory& trajectory = trajectories[i];
      jassert(trajectory.size() <= maxSteps - 1);
      String str;
      for (size_t j = 0; j < trajectory.size(); ++j)
      {
        const ObjectPtr& action = trajectory[j];
        str += action->toShortString() + T(" ");
        observeStateActionReward(context, j, stack, action, weakObjective, weight);
        executeAction(stack, action);
      }
      observeStateActionReward(context, trajectory.size(), stack, ObjectPtr(), weakObjective, weight); // finish with "yield"
      //context.informationCallback(T("Trajectory ") + String((int)i) + T(": ") + str + T(" yield"));
    }
  }

protected:
  friend class AdaptativeSequentialBuilderWeakLearnerClass;
  
  bool useVariableRelevancies;
  bool useExtendedVariables;

  void getAllTrajectoriesToBuild(const LuapeNodePtr& target, size_t budget, std::vector<Trajectory>& res)
  {
    if (!budget)
      return;

    if (target.isInstanceOf<LuapeInputNode>() || useExtendedVariables)
      res.push_back(Trajectory(1, target)); // single-step trajectory

    LuapeFunctionNodePtr functionNode = target.dynamicCast<LuapeFunctionNode>();
    if (functionNode)
    {
      size_t n = functionNode->getNumArguments();
      if (budget < n + 1)
        return;

      if (n == 1)
      {
        LuapeNodePtr argument = functionNode->getArgument(0);
        std::vector<Trajectory> trajectories;
        getAllTrajectoriesToBuild(argument, budget - n, trajectories);
        res.reserve(res.size() + trajectories.size());
        for (size_t i = 0; i < trajectories.size(); ++i)
        {
          Trajectory trajectory = trajectories[i];
          trajectory.push_back(functionNode->getFunction());
          res.push_back(trajectory);
        }
      }
      else if (n == 2)
      {
        LuapeNodePtr argument1 = functionNode->getArgument(0);
        LuapeNodePtr argument2 = functionNode->getArgument(1);

        std::vector<Trajectory> trajectories1, trajectories2;
        getAllTrajectoriesToBuild(argument1, budget - n, trajectories1);
        getAllTrajectoriesToBuild(argument2, budget - n, trajectories2);

        res.reserve(res.size() + trajectories1.size() * trajectories2.size());
        for (size_t i = 0; i < trajectories1.size(); ++i)
        {
          const Trajectory& trajectory1 = trajectories1[i];
          for (size_t j = 0; j < trajectories2.size(); ++j)
          {
            const Trajectory& trajectory2 = trajectories2[j];
            size_t size = trajectory1.size() + trajectory2.size() + 1;
            if (size <= budget)
            {
              Trajectory trajectory;
              trajectory.reserve(size);
              for (size_t i = 0; i < trajectory1.size(); ++i)
                trajectory.push_back(trajectory1[i]);
              for (size_t i = 0; i < trajectory2.size(); ++i)
                trajectory.push_back(trajectory2[i]);
              trajectory.push_back(functionNode->getFunction());
              res.push_back(trajectory);
            }
          }
        }
      }
      else
        jassert(false); // not implemented yet
    }
  }
};

///////////////////////////////////////////////////////

class AdaptativeSamplingWeakLearner : public AdaptativeSequentialBuilderWeakLearner
{
public:
  AdaptativeSamplingWeakLearner(size_t numWeakNodes, size_t maxSteps, bool useVariableRelevancies, bool useExtendedVariables)
    : AdaptativeSequentialBuilderWeakLearner(numWeakNodes, maxSteps, useVariableRelevancies, useExtendedVariables), temperature(5.0) {}
  AdaptativeSamplingWeakLearner() : temperature(0.0) {}
  
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    if (!SequentialBuilderWeakLearner::initialize(context, function))
      return false;

    for (size_t i = 0; i < function->getNumInputs(); ++i)
      pushActions.addAction(function->getInput(i));
    pushActions.normalizeProbabilities();

    for (LuapeGraphBuilderTypeSearchSpace::StateMap::const_iterator it = typeSearchSpace->getStates().begin();
          it != typeSearchSpace->getStates().end(); ++it)
    {
      const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& applyActions = it->second->getApplyActions();
      for (size_t i = 0; i < applyActions.size(); ++i)
        this->applyActions.addAction(applyActions[i].first);
    }
    applyActions.normalizeProbabilities();

    yieldActions.addAction(ObjectPtr());
    return true;
  }
 
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    AdaptativeSamplingWeakLearner& pthis = *const_cast<AdaptativeSamplingWeakLearner* >(this);

    if (useVariableRelevancies)
    {
      pthis.pushActions.updateProbabilities(temperature);
      pthis.applyActions.updateProbabilities(temperature);
      pthis.yieldActions.updateProbabilities(temperature);
    }

    // Display current policy
    context.resultCallback(T("temperature"), temperature);
    context.informationCallback(T("Temperature = ") + String(temperature));
    context.informationCallback(T("----- Push Actions -----"));
    pushActions.display(context);
    context.informationCallback(T("----- Apply Actions -----"));
    applyActions.display(context);
    context.informationCallback(T("----- Yield Action -----"));
    yieldActions.display(context);
    
    return AdaptativeSequentialBuilderWeakLearner::getCandidateWeakNodes(context, structureLearner, candidates);
  }
 
  virtual void samplingDone(ExecutionContext& context, size_t numSamplingFailures, size_t numFailuresAllowed)
  {
   /* if (numSamplingFailures > numFailuresAllowed / 10)
    {
      temperature *= 1.1;
      context.informationCallback(T("Num internal sampling failures: ") + String((int)numSamplingFailures) + T(" / ") + String((int)numFailuresAllowed));
      context.informationCallback(T("Temperature: ") + String(temperature));
    }
    else if (numSamplingFailures == 0 && temperature > 0.001)
      temperature *= 0.999999;*/
  }

  struct ActionsInfo
  {
    ActionsInfo() : Z(0.0) {}

    std::map<ObjectPtr, size_t> actionsMap;
    std::vector<ObjectPtr> actions;
    std::vector<double> weights;
    std::vector<double> probabilities;
    double Z;

    ObjectPtr sample(const RandomGeneratorPtr& random) const
    {
      jassert(actions.size() == probabilities.size());
      return actions[random->sampleWithProbabilities(probabilities, Z)];
    }

    void addAction(const ObjectPtr& action, double initialWeight = 1.0)
    {
      std::map<ObjectPtr, size_t>::const_iterator it = actionsMap.find(action);
      if (it == actionsMap.end())
      {
        actionsMap[action] = actions.size();
        actions.push_back(action);
        weights.push_back(initialWeight);
        probabilities.push_back(1.0);
        Z += 1.0;
      }
    }

    void addWeight(const ObjectPtr& action, double weight)
    {
      std::map<ObjectPtr, size_t>::const_iterator it = actionsMap.find(action);
      if (it == actionsMap.end())
        addAction(action, weight);
      else
        weights[it->second] += weight;
    }

    void updateProbabilities(double temperature)
    {
      Z = 0.0;
      for (size_t i = 0; i < probabilities.size(); ++i)
      {
        double p = weights[i];//exp(weights[i] / temperature);
        probabilities[i] = p;
        Z += p;
      }
    }

    void normalizeProbabilities()
    {
      if (Z)
      {
        for (size_t i = 0; i < probabilities.size(); ++i)
          probabilities[i] /= Z;
        Z = 1.0;
      }
    }

    void display(ExecutionContext& context) const
    {
      std::multimap<double, size_t> sortedActions;
      for (size_t i = 0; i < weights.size(); ++i)
        sortedActions.insert(std::make_pair(-weights[i], i));
      size_t i = 0;
      for (std::multimap<double, size_t>::const_iterator it = sortedActions.begin(); it != sortedActions.end() && i < 20; ++it, ++i)
      {
        size_t index = it->second;
        String description = actions[index] ? actions[index]->toShortString() : T("yield");
        double weight = weights[index];
        double probability = probabilities[index] / Z;
        context.informationCallback(T("# ") + String((int)i + 1) + T(": ") + description + T(" w = ") + String(weight, 2) + T(" p = ") + String(probability * 100.0, 2) + T("%"));
      }
    }
  };

  ActionsInfo pushActions;
  ActionsInfo applyActions;
  ActionsInfo yieldActions; // contains a single action

  bool isActionAvailable(LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr action) const
  {
    if (action == ObjectPtr())
      return typeState->hasYieldAction();
    if (action.isInstanceOf<LuapeNode>())
      return typeState->hasPushAction(action.staticCast<LuapeNode>()->getType());
    if (action.isInstanceOf<LuapeFunction>())
      return typeState->hasApplyAction(action.staticCast<LuapeFunction>());
    jassert(false);
    return false;
  }

  virtual bool sampleAction(ExecutionContext& context, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const
  {
    std::vector<double> probabilities(3, 0.0);
    double Z = 0.0;
    if (typeState->hasPushActions())
    {
      probabilities[0] = pushActions.Z;
      Z += pushActions.Z;
    }
    if (typeState->hasApplyActions())
    {
      probabilities[1] = applyActions.Z;
      Z += applyActions.Z;
    }
    if (typeState->hasYieldAction())
    {
      probabilities[2] = yieldActions.Z;
      Z += yieldActions.Z;
    }
    jassert(Z > 0.0);
    size_t actionKind = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);

    if (actionKind == 2)
    {
      res = ObjectPtr(); // yield
      return true;
    }

    // push or apply
    const ActionsInfo& actions = (actionKind == 0 ? pushActions : applyActions);
    static const size_t numFailuresAllowed = 100;
    for (size_t numFailures = 0; numFailures < numFailuresAllowed; ++numFailures)
    {
      res = actions.sample(context.getRandomGenerator());
      if (isActionAvailable(typeState, res))
        return true;
    }
    return false;
  }

  virtual void observeStateActionReward(ExecutionContext& context, size_t stepNumber, const std::vector<LuapeNodePtr>& stack, const ObjectPtr& action, double weakObjective, double weight)
  {
    if (action == ObjectPtr())
      yieldActions.addWeight(action, weakObjective * weight);
    else if (action.isInstanceOf<LuapeNode>())
      pushActions.addWeight(action, weakObjective * weight);
    else if (action.isInstanceOf<LuapeFunction>())
      applyActions.addWeight(action, weakObjective * weight);
    else
      jassertfalse;
  }

protected:
  friend class AdaptativeSamplingWeakLearnerClass;

  double temperature;
};


///////////////////////////////////////////////////////
#if 0
class AdaptativeSamplingWeakLearner : public AdaptativeSequentialBuilderWeakLearner
{
public:
  AdaptativeSamplingWeakLearner(size_t numWeakNodes, size_t maxSteps, bool useVariableRelevancies, bool useExtendedVariables)
    : AdaptativeSequentialBuilderWeakLearner(numWeakNodes, maxSteps, useVariableRelevancies, useExtendedVariables), temperature(1.0) {}
  AdaptativeSamplingWeakLearner() : temperature(0.0) {}
  
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    if (!SequentialBuilderWeakLearner::initialize(context, function))
      return false;

    stateActionStatistics.resize(typeSearchSpace->getNumStates());
    for (LuapeGraphBuilderTypeSearchSpace::StateMap::const_iterator it = typeSearchSpace->getStates().begin();
          it != typeSearchSpace->getStates().end(); ++it)
    {
      size_t stateIndex = it->second->getStateIndex();
      jassert(stateIndex < stateActionStatistics.size());
      createInitialActions(function, it->second, stateActionStatistics[stateIndex]);
    }

    //objectiveStats = new ScalarVariableRecentMeanAndVariance("weakObjective", numWeakNodes * 2);
    return true;
  }

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    for (size_t i = 0; i < stateActionStatistics.size(); ++i)
      const_cast<TypeStateActionStatistics&>(stateActionStatistics[i]).update(temperature);
    // Display current policy
    for (LuapeGraphBuilderTypeSearchSpace::StateMap::const_iterator it = typeSearchSpace->getStates().begin(); it != typeSearchSpace->getStates().end(); ++it)
    {
      LuapeGraphBuilderTypeStatePtr typeState = it->second;
      const TypeStateActionStatistics& stats = stateActionStatistics[typeState->getStateIndex()];
      if (stats.observationCount > 0.0 && stats.getTotalNumActions() > 1)
        context.informationCallback(String(stats.observationCount) + T(" -- ") + typeState->toShortString() + T("\n") + stats.toShortString());
    }
    // --
    return SequentialBuilderWeakLearner::getCandidateWeakNodes(context, structureLearner, candidates);
  }

  virtual bool sampleAction(ExecutionContext& context, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const
  {
    size_t typeStateIndex = typeState->getStateIndex();
    const TypeStateActionStatistics& stats = stateActionStatistics[typeStateIndex];
    return stats.sampleAction(context, res);
  }

  virtual void observeStateActionReward(ExecutionContext& context, size_t stepNumber, const std::vector<LuapeNodePtr>& stack, const ObjectPtr& object, double weakObjective, double weight)
  {
    LuapeGraphBuilderTypeStatePtr typeState = getTypeState(stepNumber, stack);
    if (typeState)
    {
      size_t typeStateIndex = typeState->getStateIndex();
      jassert(typeStateIndex < stateActionStatistics.size());
      stateActionStatistics[typeStateIndex].observeReward(context, object, weakObjective, weight);
    }
  }

protected:
  friend class AdaptativeSamplingWeakLearnerClass;

  //ScalarVariableRecentMeanAndVariancePtr objectiveStats;

  struct ActionStatistics
  {
    ActionStatistics() : Z(0.0) {}

    void addAction(const ObjectPtr& action)
      {m[action] = 0.0;}

    void removeAction(const ObjectPtr& action)
      {m.erase(action);}

    size_t getNumActions() const
      {return m.size();}

    void observeReward(ExecutionContext& context, const ObjectPtr& action, double weakObjective, double weight)
    {
      if (m.size() < 2)
        return;
      //context.informationCallback(T("Observe: ") + (action ? action->toShortString() : "yield") + T(" => ") + String(weakObjective) + T(" (") + String((int)examples->size()) + T(" examples)"));
      // m[action].push(weakObjective, weight);

      // copy scores from map to vector
      size_t index = 0;
      size_t actionIndex = (size_t)-1;
      DenseDoubleVectorPtr scores = new DenseDoubleVector(m.size(), 0.0);
      for (ActionMap::const_iterator it = m.begin(); it != m.end(); ++it)
      {
        scores->setValue(index, it->second);
        if (it->first == action)
          actionIndex = index;
        ++index;
      }
      jassert(actionIndex != (size_t)-1);

      // perform stochastic gradient step
      static const double learningRate = 0.1;
      double alpha = learningRate * weakObjective * weight;

      double logZ = scores->computeLogSumOfExponentials();
      for (size_t i = 0; i < scores->getNumValues(); ++i)
      {
        double score = scores->getValue(i);
        double derivative = exp(score - logZ);
        jassert(isNumberValid(derivative));
        scores->decrementValue(i, alpha * derivative);
      }
      scores->incrementValue(actionIndex, alpha);

      // copy values back to map
      index = 0;
      for (ActionMap::iterator it = m.begin(); it != m.end(); ++it)
        it->second = scores->getValue(index++);
    }

    struct SortOperator
    {
      bool operator()(const std::pair<ObjectPtr, double>& a, const std::pair<ObjectPtr, double>& b) const
        {return a.second == b.second ? a.first < b.first : a.second > b.second;}
    };

    void sortActions()
    {
      sortedActions.clear();
      for (ActionMap::const_iterator it = m.begin(); it != m.end(); ++it)
        sortedActions.push_back(std::make_pair(it->first, it->second));
      std::sort(sortedActions.begin(), sortedActions.end(), SortOperator());
      //if (sortedActions.size() > 64) // test !
      //  sortedActions.erase(sortedActions.begin() + 64, sortedActions.end());
    }

    void computeProbabilities(double temperature)
    {
      probabilities.resize(sortedActions.size());
      Z = 0.0;
      for (size_t j = 0; j < probabilities.size(); ++j)
      {
        double p = exp(sortedActions[j].second / temperature);
        Z += p;
        probabilities[j] = p;
      }
    }

    String toShortString() const
    {
      jassert(probabilities.size() == m.size() && sortedActions.size() == m.size());
      String info;
      for (size_t i = 0; i < sortedActions.size(); ++i)
      {
        ObjectPtr action = sortedActions[i].first;
        info += (action ? action->toShortString() : "yield");
        info += T(" ") + String(sortedActions[i].second) + T(", ") + String(probabilities[i] * 100.0 / Z, 1) + T("% ");
      }
      return info;
    }

    ObjectPtr sampleAction(ExecutionContext& context) const
    {
      jassert(probabilities.size() == m.size() && sortedActions.size() == m.size());
      if (m.empty())
        return ObjectPtr();
      else if (sortedActions.size() == 1)
        return sortedActions[0].first;
      else
      {
        size_t actionSortedIndex = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
        ObjectPtr res = sortedActions[actionSortedIndex].first;
        return res;
      }
    }

    // key: either LuapeNodePtr to push, or LuapeFunctionPtr to apply, or ObjectPtr() to yield

    typedef std::map<ObjectPtr, double> ActionMap;
    ActionMap m;
    std::vector< std::pair<ObjectPtr, double> > sortedActions; // from highest score to lowest score
    std::vector<double> probabilities;
    double Z;
  };

  struct TypeStateActionStatistics
  {
    TypeStateActionStatistics() : observationCount(0.0) {}

   // operation action = type (either inheriting from LuapeFunction for apply actions or from any other type for push actions, or ObjectPtr() for yield action)
    ActionStatistics operations; // operation -> statistics; 
    std::map<TypePtr, ActionStatistics> arguments; // operation -> argument -> statistics
    double observationCount;

    size_t getTotalNumActions() const
    {
      size_t res = 0;
      for (ActionStatistics::ActionMap::const_iterator it = operations.m.begin(); it != operations.m.end(); ++it)
      {
        ObjectPtr operation = it->first;
        res += operation ? arguments.find(operation)->second.getNumActions() : 1;
      }
      return res;
    }

    void update(double temperature)
    {
      operations.sortActions();
      operations.computeProbabilities(temperature);
      for (std::map<TypePtr, ActionStatistics>::iterator it = arguments.begin(); it != arguments.end(); ++it)
      {
        it->second.sortActions();
        it->second.computeProbabilities(temperature);
      }
    }

    bool sampleAction(ExecutionContext& context, ObjectPtr& res) const
    {
      ObjectPtr operation = operations.sampleAction(context);
      if (!operation)
      {
        res = operation; // yield
        return true;
      }
      std::map<TypePtr, ActionStatistics>::const_iterator it = arguments.find(operation);
      jassert(it != arguments.end());
      res = it->second.sampleAction(context);
      return res != ObjectPtr();
    }

    void observeReward(ExecutionContext& context, const ObjectPtr& action, double weakObjective, double weight)
    {
      ObjectPtr operation;
      if (action)
      {
        if (action.isInstanceOf<LuapeFunction>())
          operation = action->getClass(); // apply action
        else if (action.isInstanceOf<LuapeNode>())
          operation = action.staticCast<LuapeNode>()->getType(); // push action
        else
          jassert(false);
      }
      operations.observeReward(context, operation, weakObjective, weight);
      if (operation)
        arguments[operation].observeReward(context, action, weakObjective, weight);
      observationCount += weight;
    }

    String toShortString() const
    {
      String res;
      for (size_t i = 0; i < operations.sortedActions.size(); ++i)
      {
        ObjectPtr operation = operations.sortedActions[i].first;
        res += T("  ") + (operation ? operation->getName() : T("yield")) + T(" [") + String(operations.probabilities[i] * 100.0 / operations.Z, 1) + T("%]:\n");
        if (operation)
        {
          const ActionStatistics& args = arguments.find(operation)->second;
          res += T("    ") + args.toShortString() + T("\n");
        }
      }
      return res;
    }
  };

  std::vector<TypeStateActionStatistics> stateActionStatistics;
  double temperature;

  void createInitialActions(const LuapeInferencePtr& function, const LuapeGraphBuilderTypeStatePtr& typeState, TypeStateActionStatistics& res)
  {
    const std::vector<std::pair<TypePtr, LuapeGraphBuilderTypeStatePtr> >& pushActions = typeState->getPushActions();
    if (pushActions.size())
    {
      for (size_t i = 0; i < pushActions.size(); ++i)
      {
        TypePtr pushType = pushActions[i].first;
        res.operations.addAction(pushType);
        ActionStatistics& actions = res.arguments[pushType];
        for (size_t i = 0; i < function->getNumInputs(); ++i)
        {
          LuapeNodePtr node = function->getInput(i);
          if (node->getType() == pushType)
            actions.addAction(node);
        }
        if (!actions.getNumActions())
        {
          res.operations.removeAction(pushType);
          res.arguments.erase(pushType);
        }
      }
    }
    const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& applyActions = typeState->getApplyActions();
    if (applyActions.size())
    {
      std::set<ClassPtr> functionClasses;
      for (size_t i = 0; i < applyActions.size(); ++i)
        functionClasses.insert(applyActions[i].first->getClass());
      for (std::set<ClassPtr>::const_iterator it = functionClasses.begin(); it != functionClasses.end(); ++it)
      {
        res.operations.addAction(*it);
        ActionStatistics& actions = res.arguments[*it];
        for (size_t i = 0; i < applyActions.size(); ++i)
          if (applyActions[i].first->getClass() == *it)
            actions.addAction(applyActions[i].first);
      }
    }
    if (typeState->hasYieldAction())
      res.operations.addAction(ObjectPtr());
  }
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADAPTATIVE_SAMPLING_WEAK_H_
