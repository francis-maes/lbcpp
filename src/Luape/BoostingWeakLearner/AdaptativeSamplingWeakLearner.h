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
    : StochasticFiniteBoostingWeakLearner(numWeakNodes), maxSteps(maxSteps), temperature(0.05) {}
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
      stateActionStatistics[stateIndex].createInitialActions(function, it->second);
    }
    return true;
  }

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
  {
    for (size_t i = 0; i < stateActionStatistics.size(); ++i)
      const_cast<ActionStatistics&>(stateActionStatistics[i]).sortActions();
    return StochasticFiniteBoostingWeakLearner::getCandidateWeakNodes(context, structureLearner, candidates);
  }

  virtual LuapeNodePtr sampleWeakNode(ExecutionContext& context, const BoostingLearnerPtr& structureLearner) const
  {
    std::vector<LuapeNodePtr> stack;
    LuapeGraphBuilderTypeStatePtr typeState;

    for (size_t i = 0; i < maxSteps; ++i)
    {
      // Retrieve type-state index
      size_t typeStateIndex = getTypeStateIndex(i, stack);

      // Compute probabilities
      const ActionStatistics& stats = stateActionStatistics[typeStateIndex];

      std::vector<double> probabilities(stats.sortedActions.size());
      double Z = 0.0;
      for (size_t j = 0; j < probabilities.size(); ++j)
      {
        double p = exp(stats.sortedActions[j].second / temperature);
        Z += p;
        probabilities[j] = p;
      }

      // Sample action
      ObjectPtr action;
      size_t numFailuresAllowed = 20;
      for (size_t i = 0; i < numFailuresAllowed; ++i)
      {
        size_t actionSortedIndex = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
        action = stats.sortedActions[actionSortedIndex].first;
        if (isActionAvailable(action, stack))
          break;
      }

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
    std::vector<Trajectory> trajectories;
    getAllTrajectoriesToBuild(weakNode, maxSteps - 1, trajectories);
    jassert(trajectories.size());
   // context.informationCallback(String((int)trajectories.size()) + T(" trajectories to build ") + weakNode->toShortString() + T(" in ") + String((int)maxSteps - 1) + T(" steps"));

    weakObjective *= examples->size() / (double)structureLearner->getTrainingCache()->getNumSamples();

    for (size_t i = 0; i < trajectories.size(); ++i)
    {
      std::vector<LuapeNodePtr> stack;
      size_t stepNumber = 0;
      const Trajectory& trajectory = trajectories[i];
      jassert(trajectory.size() <= maxSteps - 1);
      String str;
      for (size_t j = 0; j < trajectory.size(); ++j)
      {
        const ObjectPtr& action = trajectory[j];
        str += action->toShortString() + T(" ");
        observeStateActionReward(stepNumber, stack, action, examples, weakObjective);
        executeAction(stack, action);
      }
      //context.informationCallback(T("Trajectory: ") + str + T(" yield"));
      observeStateActionReward(stepNumber, stack, ObjectPtr(), examples, weakObjective);
    }
  }

  void observeStateActionReward(size_t stepNumber, const std::vector<LuapeNodePtr>& stack, const ObjectPtr& object, const IndexSetPtr& examples, double weakObjective)
  {
    size_t typeStateIndex = getTypeStateIndex(stepNumber, stack);
    if (typeStateIndex != (size_t)-1)
    {
      jassert(typeStateIndex < stateActionStatistics.size());
      ActionStatistics& stats = stateActionStatistics[typeStateIndex];
      if (!object.isInstanceOf<LuapeFunctionNode>()) // Test !
        stats.m[object].push(weakObjective, examples->size());
    }
  }

protected:
  friend class AdaptativeSamplingWeakLearnerClass;

  size_t maxSteps;

  LuapeNodeUniversePtr universe;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;

  struct ActionStatistics
  {
    void createInitialActions(const LuapeInferencePtr& function, const LuapeGraphBuilderTypeStatePtr& typeState)
    {
      if (typeState->hasPushActions())
      {
        for (size_t i = 0; i < function->getNumInputs(); ++i)
        {
          LuapeNodePtr node = function->getInput(i);
          if (typeState->canTypeBePushed(node->getType()))
            addAction(node);
        }
      }
      if (typeState->hasApplyActions())
      {
        const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& apply = typeState->getApplyActions();
        for (size_t i = 0; i < apply.size(); ++i)
          addAction(apply[i].first);
      }
      if (typeState->hasYieldAction())
        addAction(ObjectPtr());
    }

    void addAction(const ObjectPtr& action)
    {
      m[action] = ScalarVariableMeanAndVariance();
      sortedActions.push_back(std::make_pair(action, 0.0));
    }

    struct SortOperator
    {
      bool operator()(const std::pair<ObjectPtr, double>& a, const std::pair<ObjectPtr, double>& b) const
        {return a.second == b.second ? a.first < b.first : a.second > b.second;}
    };

    void sortActions()
    {
      sortedActions.clear();
      for (std::map<ObjectPtr, ScalarVariableMeanAndVariance>::const_iterator it = m.begin(); it != m.end(); ++it)
        sortedActions.push_back(std::make_pair(it->first, it->second.getMean()));
      std::sort(sortedActions.begin(), sortedActions.end(), SortOperator());
      //if (sortedActions.size() > 64) // test !
      //  sortedActions.erase(sortedActions.begin() + 64, sortedActions.end());
    }

    size_t getNumActions() const
      {return m.size();}

    // key: either LuapeNodePtr to push, or LuapeFunctionPtr to apply, or ObjectPtr() to yield
    std::map<ObjectPtr, ScalarVariableMeanAndVariance> m;

    std::vector< std::pair<ObjectPtr, double> > sortedActions; // from highest score to lowest score
  };

  std::vector<ActionStatistics> stateActionStatistics;
  double temperature;

  static bool isActionAvailable(ObjectPtr action, const std::vector<LuapeNodePtr>& stack)
  {
    return !action || !action.isInstanceOf<LuapeFunction>() ||
      action.staticCast<LuapeFunction>()->acceptInputsStack(stack);
  }

  size_t getTypeStateIndex(size_t stepNumber, const std::vector<LuapeNodePtr>& stack) const
  {
    std::vector<TypePtr> typeStack(stack.size());
    for (size_t j = 0; j < typeStack.size(); ++j)
      typeStack[j] = stack[j]->getType();
    LuapeGraphBuilderTypeStatePtr typeState = typeSearchSpace->getState(stepNumber, typeStack);
    return typeState ? typeState->getStateIndex() : (size_t)-1;
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
