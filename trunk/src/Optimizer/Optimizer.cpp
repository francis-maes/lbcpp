/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Implementation file of          |
| Author  : Arnaud Schoofs                 | Optimizer, OptimizerState and   |
| Started : 11/04/2011                     | OptimizerContext                |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Optimizer/Optimizer.h>
#include <lbcpp/Optimizer/OptimizerState.h>

using namespace lbcpp;

Variable Optimizer::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  OptimizerStatePtr state = createOptimizerState(context); // TODO: Load a partially learn state if exists.
  context.enterScope(T("Optimizing - ") + toString());
  jassert(inputs[0].getObjectAndCast<Function>(context));
  const FunctionPtr objectiveFunction = inputs[0].getObjectAndCast<Function>(context);
  const FunctionPtr validationFunction = getNumInputs() > 1 ? inputs[1].getObjectAndCast<Function>(context) : FunctionPtr();
  OptimizerStatePtr res = optimize(context, state, objectiveFunction, validationFunction);
  context.resultCallback(T("bestParameters"), state->getBestParameters());
  context.resultCallback(T("bestScore"), state->getBestScore());
  context.leaveScope(res);
  return res;
}

/*
void OptimizerState::autoSaveToFile(ExecutionContext& context, bool force)
{
  ScopedLock _(lock);
  double time = Time::getMillisecondCounter() / 1000.0;
  if (autoSaveStateFrequency > 0.0 && (force || (time - lastSaveTime >= autoSaveStateFrequency)))
  {
    lastSaveTime = time;
    if (context.getFile(T("optimizerState.xml")).existsAsFile())
      context.getFile(T("optimizerState.xml")).copyFileTo(context.getFile(T("optimizerState_backup.xml")));
    saveToFile(context, context.getFile(T("optimizerState.xml")));
  }
}
*/
