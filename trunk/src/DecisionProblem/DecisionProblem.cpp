/*-----------------------------------------.---------------------------------.
| Filename: DecisionProblem.cpp            | Sequential Decision Problem     |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/DecisionProblem/DecisionProblem.h>
#include <lbcpp/Lua/Lua.h>
using namespace lbcpp;

void DecisionProblemState::performTrajectory(const ContainerPtr& actions, double& sumOfRewards, size_t maxSteps)
{
  ExecutionContext& context = defaultExecutionContext();
  jassert(actions->getElementsType() == getActionType());
  size_t n = actions->getNumElements();
  if (maxSteps && n > maxSteps)
    n = maxSteps;
  for (size_t i = 0; i < n; ++i)
  {
    double reward;
    performTransition(context, actions->getElement(i), reward);
    sumOfRewards += reward;
  }
}

bool DecisionProblemState::checkTrajectoryValidity(ExecutionContext& context, const ContainerPtr& trajectory) const
{
  jassert(trajectory->getElementsType()->inheritsFrom(getActionType()));
  DecisionProblemStatePtr state = cloneAndCast<DecisionProblemState>();
  size_t n = trajectory->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    Variable action = trajectory->getElement(i);

    ContainerPtr availableActions = state->getAvailableActions();
    if (!availableActions || !availableActions->getNumElements())
    {
      context.errorCallback(toShortString(), T("Reached final state"));
      return false;
    }

    size_t j;
    for (j = 0; j < availableActions->getNumElements(); ++j)
      if (availableActions->getElement(j) == action)
        break;
    if (j == availableActions->getNumElements())
    {
      context.errorCallback(toShortString(), T("Action ") + action.toShortString() + T(" is not available in this state"));
      return false;
    }

    double reward;
    state->performTransition(context, action, reward);
  }
  //context.resultCallback(state->getName(), state);
  return true;
}


int DecisionProblemState::getActions(LuaState& state)
{
  DecisionProblemStatePtr dpstate = state.checkObject(1, decisionProblemStateClass).staticCast<DecisionProblemState>();
  ContainerPtr actions = dpstate->getAvailableActions();
  state.pushObject(actions);
  return 1;
}

int DecisionProblemState::isFinal(LuaState& state)
{
  DecisionProblemStatePtr dpstate = state.checkObject(1, decisionProblemStateClass).staticCast<DecisionProblemState>();
  state.pushBoolean(dpstate->isFinalState());
  return 1;
}

int DecisionProblemState::performTransition(LuaState& state)
{
  DecisionProblemStatePtr dpstate = state.checkObject(1, decisionProblemStateClass).staticCast<DecisionProblemState>();
  Variable action = state.checkVariable(2);
  double reward;
  dpstate->performTransition(state.getContext(), action, reward);
  state.pushNumber(reward);
  return 1;
}

/*
** DecisionProblem
*/
ClassPtr DecisionProblem::getStateClass() const
{
  jassert(initialStateSampler);
  if (!initialStateSampler->isInitialized())
    initialStateSampler->initialize(defaultExecutionContext(), randomGeneratorClass);
  return initialStateSampler->getOutputType();
}

DecisionProblemStatePtr DecisionProblem::sampleInitialState(ExecutionContext& context) const
{
  Variable res = initialStateSampler->compute(context, context.getRandomGenerator());
  return res.exists() ? res.getObjectAndCast<DecisionProblemState>() : DecisionProblemStatePtr();
}

ContainerPtr DecisionProblem::sampleInitialStates(ExecutionContext& context, size_t count) const
{
  ObjectVectorPtr res = new ObjectVector(getStateClass(), count);
  for (size_t i = 0; i < count; ++i)
    res->set(i, sampleInitialState(context));
  return res;
}

double DecisionProblem::getMaxCumulativeReward() const
{
  double maxReward = getMaxReward();
  if (discount == 1.0)
    return maxReward * horizon;
  else
    return maxReward * (1.0 - pow(discount, (double)horizon)) / (1.0 - discount);
}

void DecisionProblem::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  Object::clone(context, t);
  const DecisionProblemPtr& target = t.staticCast<DecisionProblem>();
  target->initialStateSampler = initialStateSampler;
  target->discount = discount;
  target->horizon = horizon;
}
