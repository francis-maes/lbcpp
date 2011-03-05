/*-----------------------------------------.---------------------------------.
| Filename: SequentialDecisionProblem.cpp  | Sequential Decision System      |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "SequentialDecisionProblem.h"
using namespace lbcpp;

SequentialDecisionProblem::SequentialDecisionProblem(const FunctionPtr& initialStateSampler, const FunctionPtr& transitionFunction, const FunctionPtr& rewardFunction)
  : initialStateSampler(initialStateSampler), transitionFunction(transitionFunction), rewardFunction(rewardFunction)
{
}

bool SequentialDecisionProblem::initialize(ExecutionContext& context)
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

void SequentialDecisionProblem::getAvailableActions(const Variable& state, std::vector<Variable>& actions) const
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
