/*-----------------------------------------.---------------------------------.
| Filename: DecisionProblem.cpp            | Sequential Decision Problem     |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "DecisionProblem.h"
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

ClassPtr DecisionProblem::getStateClass() const
{
  if (!initialStateSampler->isInitialized())
    initialStateSampler->initialize(defaultExecutionContext(), randomGeneratorClass);
  return initialStateSampler->getOutputType();
}

DecisionProblemStatePtr DecisionProblem::sampleInitialState(ExecutionContext& context, RandomGeneratorPtr random) const
{
  Variable res = initialStateSampler->compute(context, random);
  return res.exists() ? res.getObjectAndCast<DecisionProblemState>() : DecisionProblemStatePtr();
}

ContainerPtr DecisionProblem::sampleInitialStates(ExecutionContext& context, RandomGeneratorPtr random, size_t count) const
{
  ObjectVectorPtr res = new ObjectVector(getStateClass(), count);
  for (size_t i = 0; i < count; ++i)
    res->set(i, sampleInitialState(context, random));
  return res;
}
