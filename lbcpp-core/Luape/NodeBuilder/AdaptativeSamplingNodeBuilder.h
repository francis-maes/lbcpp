/*-----------------------------------------.---------------------------------.
| Filename: AdaptativeSamplingNodeBuilder.h| Adaptative Sampling Weak Learner|
| Author  : Francis Maes                   |                                 |
| Started : 20/12/2011 16:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_ADAPTATIVE_SAMPLING_H_
# define LBCPP_LUAPE_NODE_BUILDER_ADAPTATIVE_SAMPLING_H_

# include "NodeBuilderDecisionProblem.h"
# include <lbcpp/Luape/LuapeLearner.h>
# include <algorithm>

namespace lbcpp
{

class AdaptativeSequentialNodeBuilder : public SequentialNodeBuilder
{
public:
  AdaptativeSequentialNodeBuilder(size_t numNodes, size_t complexity, bool useVariableRelevancies, bool useExtendedVariables)
    : SequentialNodeBuilder(numNodes, complexity), useVariableRelevancies(useVariableRelevancies), useExtendedVariables(useExtendedVariables) {}
  AdaptativeSequentialNodeBuilder() : useVariableRelevancies(false), useExtendedVariables(false) {}

  typedef std::vector<ObjectPtr> Trajectory;

  virtual void observeStateActionReward(ExecutionContext& context, size_t stepNumber, const std::vector<ExpressionPtr>& stack, const ObjectPtr& object, double weakObjective, double weight) = 0;
  
  // FIXME: this function is never called !!!
  void observeBestWeakNode(ExecutionContext& context, const LuapeInferencePtr& problem, const ExpressionPtr& weakNode, const IndexSetPtr& examples, double weakObjective)
  {
    if (weakObjective == -DBL_MAX)
      return;
    jassert(isNumberValid(weakObjective));

    double weight = examples->size() / (double)problem->getTrainingCache()->getNumSamples();
    weakObjective /= weight;

    // normalize objective
    /*context.informationCallback(T("Edge: ") + String(weakObjective) + T(", Weight: ") + String(weight) + T(" Stats: ") + objectiveStats->toShortString() + T(" Node = ") + weakNode->toShortString());
    objectiveStats->push(weakObjective); // FIXME: weight is not taken into account anymore (with ScalarVariableRecentMean)
    if (objectiveStats->getStandardDeviation() == 0.0)
      return;*/

    ExpressionPtr node = weakNode;
    if (node.isInstanceOf<ConstantExpression>())
      return; // those node cannot be produced by this policy
    if (node.isInstanceOf<FunctionExpression>() && node.staticCast<FunctionExpression>()->getFunction()->getClassName() == T("StumpFunction"))
      node = node.staticCast<FunctionExpression>()->getArgument(0);

    std::vector<Trajectory> trajectories;
    getAllTrajectoriesToBuild(node, complexity - 1, trajectories);
    jassert(trajectories.size());
    context.informationCallback(String((int)trajectories.size()) + T(" trajectories to build ") + node->toShortString() + T(" in ") + String((int)complexity - 1) + T(" steps"));

    for (size_t i = 0; i < trajectories.size(); ++i)
    {
      //context.informationCallback("Start of trajectory " + String((int)i));
      std::vector<ExpressionPtr> stack;
      const Trajectory& trajectory = trajectories[i];
      jassert(trajectory.size() <= complexity - 1);
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
  friend class AdaptativeSequentialNodeBuilderClass;
  
  bool useVariableRelevancies;
  bool useExtendedVariables;

  void getAllTrajectoriesToBuild(const ExpressionPtr& target, size_t budget, std::vector<Trajectory>& res)
  {
    if (!budget)
      return;

    if (target.isInstanceOf<VariableExpression>() || useExtendedVariables)
      res.push_back(Trajectory(1, target)); // single-step trajectory

    FunctionExpressionPtr functionNode = target.dynamicCast<FunctionExpression>();
    if (functionNode)
    {
      size_t n = functionNode->getNumArguments();
      if (budget < n + 1)
        return;

      if (n == 1)
      {
        ExpressionPtr argument = functionNode->getArgument(0);
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
        ExpressionPtr argument1 = functionNode->getArgument(0);
        ExpressionPtr argument2 = functionNode->getArgument(1);

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

class AdaptativeSamplingNodeBuilder : public AdaptativeSequentialNodeBuilder
{
public:
  AdaptativeSamplingNodeBuilder(size_t numWeakNodes, size_t maxSteps, bool useVariableRelevancies, bool useExtendedVariables)
    : AdaptativeSequentialNodeBuilder(numWeakNodes, maxSteps, useVariableRelevancies, useExtendedVariables), temperature(5.0) {}
  AdaptativeSamplingNodeBuilder() : temperature(0.0) {}
  
  void initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    for (size_t i = 0; i < function->getNumInputs(); ++i)
      pushActions.addAction(function->getInput(i));
    pushActions.normalizeProbabilities();

    for (LuapeGraphBuilderTypeSearchSpace::StateMap::const_iterator it = typeSearchSpace->getStates().begin();
          it != typeSearchSpace->getStates().end(); ++it)
    {
      const std::vector<std::pair<FunctionPtr, LuapeGraphBuilderTypeStatePtr> >& applyActions = it->second->getApplyActions();
      for (size_t i = 0; i < applyActions.size(); ++i)
        this->applyActions.addAction(applyActions[i].first);
    }
    applyActions.normalizeProbabilities();

    yieldActions.addAction(ObjectPtr());
  }
 
  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<ExpressionPtr>& res)
  {
    jassert(false); // initialize should only be called once
    initialize(context, function);

    if (useVariableRelevancies)
    {
      pushActions.updateProbabilities(temperature);
      applyActions.updateProbabilities(temperature);
      yieldActions.updateProbabilities(temperature);
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
    
    return AdaptativeSequentialNodeBuilder::buildNodes(context, function, maxCount, res);
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
    if (action.isInstanceOf<Expression>())
      return typeState->hasPushAction(action.staticCast<Expression>()->getType());
    if (action.isInstanceOf<Function>())
      return typeState->hasApplyAction(action.staticCast<Function>());
    jassert(false);
    return false;
  }

  virtual bool sampleAction(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const
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

  virtual void observeStateActionReward(ExecutionContext& context, size_t stepNumber, const std::vector<ExpressionPtr>& stack, const ObjectPtr& action, double weakObjective, double weight)
  {
    if (action == ObjectPtr())
      yieldActions.addWeight(action, weakObjective * weight);
    else if (action.isInstanceOf<Expression>())
      pushActions.addWeight(action, weakObjective * weight);
    else if (action.isInstanceOf<Function>())
      applyActions.addWeight(action, weakObjective * weight);
    else
      jassertfalse;
  }

protected:
  friend class AdaptativeSamplingNodeBuilderClass;

  double temperature;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADAPTATIVE_SAMPLING_WEAK_H_
