/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Implementation file of          |
| Author  : Julien Becker                  | Optimizer, OptimizerState and   |
| Started : 22/08/2011 13:43               | OptimizerContext                |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Optimizer/Optimizer.h>
#include <lbcpp/Optimizer/OptimizerState.h>

using namespace lbcpp;

Variable Optimizer::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  OptimizerStatePtr state = loadOptimizerState(context);
  if (!state)
    state = createOptimizerState(context);
  context.enterScope(T("Optimizing - ") + toString());
  jassert(inputs[0].getObjectAndCast<OptimizationProblem>(context));
  OptimizationProblemPtr problem = inputs[0].getObjectAndCast<OptimizationProblem>(context);
  OptimizerStatePtr res = optimize(context, state, problem);
  context.resultCallback(T("bestParameters"), state->getBestParameters());
  context.resultCallback(T("bestScore"), state->getBestScore());
  context.leaveScope(res);
  return res;
}

void Optimizer::saveOptimizerState(ExecutionContext& context, const OptimizerStatePtr& state) const
{
  if (optimizerStateFile == File::nonexistent)
    return;
  const File swapFile = optimizerStateFile.getParentDirectory().getChildFile(T(".") + optimizerStateFile.getFileName());
  optimizerStateFile.copyFileTo(swapFile);
  state->saveToFile(context, optimizerStateFile);
  //swapFile.deleteFile(); // in case of ...
}

OptimizerStatePtr Optimizer::loadOptimizerState(ExecutionContext& context) const
{
  if (optimizerStateFile == File::nonexistent || !optimizerStateFile.exists())
    return OptimizerStatePtr();
  return Object::createFromFile(context, optimizerStateFile).dynamicCast<OptimizerState>();
}

int Optimizer::__call(LuaState& state)
  {return Function::__call(state);} // this might be specialized in the future
