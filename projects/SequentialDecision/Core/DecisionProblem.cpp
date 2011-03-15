/*-----------------------------------------.---------------------------------.
| Filename: DecisionProblem.cpp  | Sequential Decision System      |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "DecisionProblem.h"
using namespace lbcpp;

DecisionProblem::DecisionProblem(const FunctionPtr& initialStateSampler, const FunctionPtr& transitionFunction, const FunctionPtr& rewardFunction, double discount)
  : initialStateSampler(initialStateSampler), transitionFunction(transitionFunction), rewardFunction(rewardFunction), discount(discount)
{
}

bool DecisionProblem::initialize(ExecutionContext& context)
{
  if (!initialStateSampler || !transitionFunction || !rewardFunction)
  {
    context.errorCallback(T("Missing function"));
    return false;
  }
  if (!initialStateSampler->initialize(context, (TypePtr)randomGeneratorClass))
    return false;
  
  stateType = initialStateSampler->getOutputType();

  if (transitionFunction->getNumRequiredInputs() != 2 || 
    !stateType->inheritsFrom(transitionFunction->getRequiredInputType(0, 2)))
  {
    context.errorCallback(T("Wrong prototype for transition function"));
    return false;
  }
  
  actionType = transitionFunction->getRequiredInputType(1, 2);
  if (!transitionFunction->initialize(context, stateType, actionType))
    return false;
  if (!transitionFunction->getOutputType()->inheritsFrom(stateType))
  {
    context.errorCallback(T("Unrecognized output type for the transition function"));
    return false;
  }

  if (rewardFunction->getNumRequiredInputs() != 2 ||
      !stateType->inheritsFrom(rewardFunction->getRequiredInputType(0, 2)) ||
      !actionType->inheritsFrom(rewardFunction->getRequiredInputType(1, 2)))
  {
    context.errorCallback(T("Wrong prototype for reward function"));
    return false;
  }

  if (!rewardFunction->initialize(context, stateType, actionType))
    return false;

  if (!rewardFunction->getOutputType()->inheritsFrom(doubleType))
  {
    context.errorCallback(T("Reward function does not return scalar values"));
    return false;
  }

  return true;
}

void DecisionProblem::getAvailableActions(const Variable& state, std::vector<Variable>& actions) const
{
  jassert(stateType && actionType);
  if (actionType == booleanType)
  {
    actions.resize(2);
    actions[0] = Variable(false);
    actions[1] = Variable(true);
    return;
  }

  jassert(false);
}

Variable DecisionProblem::computeFinalState(ExecutionContext& context, const Variable& initialState, const ContainerPtr& trajectory) const
{
  if (!context.checkInheritance(trajectory->getElementsType(), actionType))
    return Variable();

  Variable state = initialState;
  size_t n = trajectory->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    state = computeTransition(state, trajectory->getElement(i));
    if (!state.exists())
      break;
  }
  return state;
}

bool DecisionProblem::checkTrajectoryValidity(ExecutionContext& context, const Variable& initialState, const ContainerPtr& trajectory) const
{
  Variable state = initialState;
  size_t n = trajectory->getNumElements();
  
  for (size_t i = 0; i < n; ++i)
  {
    Variable action = trajectory->getElement(i);

    std::vector<Variable> availableActions;
    getAvailableActions(state, availableActions);
    size_t j;
    for (j = 0; j < availableActions.size(); ++j)
      if (availableActions[j] == action)
        break;
    if (j == availableActions.size())
    {
      context.errorCallback(state.toShortString(), T("Action ") + action.toShortString() + T(" is not available in this state"));
      return false;
    }

    state = computeTransition(state, action);
    if (!state.exists())
    {
      context.errorCallback(state.toShortString(), T("Reached final state"));
      return false;
    }
  }
  return true;
}
