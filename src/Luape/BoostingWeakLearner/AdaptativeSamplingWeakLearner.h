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
# include "LuapeGraphBuilder.h"
# include <algorithm>

namespace lbcpp
{

class AdaptativeSamplingWeakLearner : public StochasticFiniteBoostingWeakLearner
{
public:
  AdaptativeSamplingWeakLearner(size_t numWeakNodes, size_t maxSteps)
    : StochasticFiniteBoostingWeakLearner(numWeakNodes), maxSteps(maxSteps), temperature(0.1) {}
  AdaptativeSamplingWeakLearner() {}
  
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    universe = function->getUniverse();

    typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(function, maxSteps);
    typeSearchSpace->pruneStates(context);
    typeSearchSpace->assignStateIndices(context);

    stateActionStatistics.resize(typeSearchSpace->getNumStates());
    for (LuapeGraphBuilderTypeSearchSpace::StateMap::const_iterator it = typeSearchSpace->getStates().begin();
          it != typeSearchSpace->getStates().end(); ++it)
    {
      size_t stateIndex = it->second->getStateIndex();
      jassert(stateIndex < stateActionStatistics.size());
      createInitialActions(function, it->second, stateActionStatistics[stateIndex]);
    }

    objectiveStats = new ScalarVariableStatistics("weakObjective");
    return true;
  }

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    for (size_t i = 0; i < stateActionStatistics.size(); ++i)
      const_cast<TypeStateActionStatistics&>(stateActionStatistics[i]).update(temperature);
    return StochasticFiniteBoostingWeakLearner::getCandidateWeakNodes(context, structureLearner, candidates);
  }

  virtual LuapeNodePtr sampleWeakNode(ExecutionContext& context, const BoostingLearnerPtr& structureLearner) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    std::vector<LuapeNodePtr> stack;
    LuapeGraphBuilderTypeStatePtr typeState;

    for (size_t i = 0; i < maxSteps; ++i)
    {
      // Retrieve type-state index
      LuapeGraphBuilderTypeStatePtr typeState = getTypeState(i, stack);
      jassert(typeState);
      size_t typeStateIndex = typeState->getStateIndex();
      const TypeStateActionStatistics& stats = stateActionStatistics[typeStateIndex];

      // Sample action
      ObjectPtr action;
      size_t numFailuresAllowed = 100;
      size_t numFailures;
      for (numFailures = 0; numFailures < numFailuresAllowed; ++numFailures)
        if (stats.sampleAction(context, action) && isActionAvailable(action, stack))
          break;
      if (numFailures == numFailuresAllowed)
        return LuapeNodePtr();

      // Execute action
      executeAction(stack, action);
      if (!action)
        return stack[0]; // yield action
    }

    return LuapeNodePtr();
  }

  typedef std::vector<ObjectPtr> Trajectory;

  virtual void observeObjectiveValue(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, double weakObjective)
  {
    double weight = (examples->size() / (double)structureLearner->getTrainingCache()->getNumSamples());
    weakObjective *= weight;

    // normalize objective
    objectiveStats->push(weakObjective, weight);
    if (objectiveStats->getStandardDeviation() == 0.0)
      return;
    if (weakNode.isInstanceOf<LuapeConstantNode>())
      return; // those node cannot be produced by this policy

    double normalizedObjective = (weakObjective - objectiveStats->getMean()) / objectiveStats->getStandardDeviation();

    std::vector<Trajectory> trajectories;
    getAllTrajectoriesToBuild(weakNode, maxSteps - 1, trajectories);
    jassert(trajectories.size());
    context.informationCallback(String((int)trajectories.size()) + T(" trajectories to build ") + weakNode->toShortString() + T(" in ") + String((int)maxSteps - 1) + T(" steps"));

    for (size_t i = 0; i < trajectories.size(); ++i)
    {
      context.informationCallback("Start of trajectory " + String((int)i));
      std::vector<LuapeNodePtr> stack;
      const Trajectory& trajectory = trajectories[i];
      jassert(trajectory.size() <= maxSteps - 1);
      String str;
      for (size_t j = 0; j < trajectory.size(); ++j)
      {
        const ObjectPtr& action = trajectory[j];
        str += action->toShortString() + T(" ");
        observeStateActionReward(context, j, stack, action, normalizedObjective, weight);
        executeAction(stack, action);
      }
      observeStateActionReward(context, trajectory.size(), stack, ObjectPtr(), normalizedObjective, weight); // finish with "yield"
      context.informationCallback(T("Trajectory ") + String((int)i) + T(": ") + str + T(" yield"));
    }
  }

  void observeStateActionReward(ExecutionContext& context, size_t stepNumber, const std::vector<LuapeNodePtr>& stack, const ObjectPtr& object, double weakObjective, double weight)
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

  size_t maxSteps;

  LuapeNodeUniversePtr universe;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
  ScalarVariableStatisticsPtr objectiveStats;

  struct ActionStatistics
  {
    ActionStatistics() : Z(0.0) {}

    void addAction(const ObjectPtr& action)
      {m[action] = ScalarVariableMeanAndVariance();}

    void removeAction(const ObjectPtr& action)
      {m.erase(action);}

    size_t getNumActions() const
      {return m.size();}

    void observeReward(ExecutionContext& context, const ObjectPtr& action, double weakObjective, double weight)
    {
      //context.informationCallback(T("Observe: ") + (action ? action->toShortString() : "yield") + T(" => ") + String(weakObjective) + T(" (") + String((int)examples->size()) + T(" examples)"));
      m[action].push(weakObjective, weight);
    }

    struct SortOperator
    {
      bool operator()(const std::pair<ObjectPtr, double>& a, const std::pair<ObjectPtr, double>& b) const
        {return a.second == b.second ? a.first < b.first : a.second > b.second;}
    };

    double getScore(const ScalarVariableMeanAndVariance& variable)
    {
      double c = variable.getCount();
      return c ? (variable.getSum() - 0.0) / variable.getCount() : 5.0;
    }

    void sortActions()
    {
      sortedActions.clear();
      for (std::map<ObjectPtr, ScalarVariableMeanAndVariance>::const_iterator it = m.begin(); it != m.end(); ++it)
        sortedActions.push_back(std::make_pair(it->first, getScore(it->second)));
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

    ObjectPtr sampleAction(ExecutionContext& context) const
    {
      jassert(probabilities.size() == m.size() && sortedActions.size() == m.size());
      String info;
      for (size_t i = 0; i < sortedActions.size(); ++i)
      {
        ObjectPtr action = sortedActions[i].first;
        info += (action ? action->toShortString() : "yield");
        info += T(" ") + String(probabilities[i] * 100.0 / Z, 1) + T("% ");
      }

      if (m.empty())
        return ObjectPtr();
      else if (sortedActions.size() == 1)
        return sortedActions[0].first;
      else
      {
        size_t actionSortedIndex = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
        ObjectPtr res = sortedActions[actionSortedIndex].first;
        context.informationCallback(info + T(" ==> ") + (res ? res->toShortString() : "yield"));
        return res;
      }
    }

    // key: either LuapeNodePtr to push, or LuapeFunctionPtr to apply, or ObjectPtr() to yield
    std::map<ObjectPtr, ScalarVariableMeanAndVariance> m;
    std::vector< std::pair<ObjectPtr, double> > sortedActions; // from highest score to lowest score
    std::vector<double> probabilities;
    double Z;
  };

  struct TypeStateActionStatistics
  {
   // operation action = type (either inheriting from LuapeFunction for apply actions or from any other type for push actions, or ObjectPtr() for yield action)
    ActionStatistics operations; // operation -> statistics; 
    std::map<TypePtr, ActionStatistics> arguments; // operation -> argument -> statistics

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
        res = operation;
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
          operation = action->getClass(); // yield action
        else if (action.isInstanceOf<LuapeNode>())
          operation = action.staticCast<LuapeNode>()->getType(); // push action
        else
          jassert(false);
      }
      operations.observeReward(context, operation, weakObjective, weight);
      if (operation)
        arguments[operation].observeReward(context, action, weakObjective, weight);
    }
  };

  std::vector<TypeStateActionStatistics> stateActionStatistics;
  double temperature;

  static bool isActionAvailable(ObjectPtr action, const std::vector<LuapeNodePtr>& stack)
  {
    return !action || !action.isInstanceOf<LuapeFunction>() ||
      action.staticCast<LuapeFunction>()->acceptInputsStack(stack);
  }

  LuapeGraphBuilderTypeStatePtr getTypeState(size_t stepNumber, const std::vector<LuapeNodePtr>& stack) const
  {
    std::vector<TypePtr> typeStack(stack.size());
    for (size_t j = 0; j < typeStack.size(); ++j)
      typeStack[j] = stack[j]->getType();
    return typeSearchSpace->getState(stepNumber, typeStack);
  }

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

  void executeAction(std::vector<LuapeNodePtr>& stack, const ObjectPtr& action) const
  {
    // Execute action
    if (action)
    {
      if (action.isInstanceOf<LuapeNode>())
        stack.push_back(action);   // push action
      else
      {
        // apply action
        LuapeFunctionPtr function = action.staticCast<LuapeFunction>();
        size_t n = function->getNumInputs();
        jassert(stack.size() >= n && n > 0);
        std::vector<LuapeNodePtr> inputs(n);
        for (size_t i = 0; i < n; ++i)
          inputs[i] = stack[stack.size() - n + i];
        stack.erase(stack.begin() + stack.size() - n, stack.end());
        stack.push_back(universe->makeFunctionNode(function, inputs));
      }
    }
    else
    {
      // yield action
      jassert(stack.size() == 1);
    }
  }

  void getAllTrajectoriesToBuild(const LuapeNodePtr& target, size_t budget, std::vector<Trajectory>& res)
  {
    if (!budget)
      return;

    if (target.isInstanceOf<LuapeInputNode>()) // TMP: restrict to input nodes
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


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADAPTATIVE_SAMPLING_WEAK_H_
