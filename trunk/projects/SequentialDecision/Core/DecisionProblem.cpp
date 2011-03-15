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

void DecisionProblemState::performTrajectory(const ContainerPtr& actions, double& sumOfRewards)
{
  jassert(actions->getElementsType() == getActionType());
  size_t n = actions->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    double reward;
    performTransition(actions->getElement(i), reward);
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
    state->performTransition(action, reward);
   
  }
  return true;
}
