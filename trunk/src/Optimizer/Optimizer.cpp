/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Implementation file of          |
| Author  : Julien Becker                  | Optimizer, OptimizerState and   |
| Started : 22/08/2011 13:43               | OptimizerContext                |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Optimizer/OptimizerState.h>
#include <lbcpp/Optimizer/Optimizer.h>
using namespace lbcpp;

/*
** OptimizerState
*/
void OptimizerState::submitSolution(const Variable& solution, double score)
{
  if (score < bestScore || !bestSolution.exists())
  {
    bestScore = score;
    bestSolution = solution;
  }
}

Variable OptimizerState::finishIteration(ExecutionContext& context, const OptimizationProblemPtr& problem, size_t iteration, double bestIterationScore, const Variable& bestIterationSolution)
{
  submitSolution(bestIterationSolution, bestIterationScore);
  context.resultCallback("iteration", iteration);
  context.resultCallback("bestIterationScore", bestIterationScore);
  context.resultCallback("bestIterationSolution", bestIterationSolution);
  FunctionPtr validation = problem->getValidation();
  double validationScore = 0.0;
  if (validation)
  {
    validationScore = validation->compute(context, bestIterationSolution).toDouble();
    context.resultCallback("bestIterationValidation", validationScore);
  }
  context.resultCallback("bestScore", bestScore);
  context.resultCallback("bestSolution", bestSolution);

  return validation ? Variable(new Pair(bestIterationScore, validationScore)) : Variable(bestIterationScore);
}

/*
** Optimizer
*/
Variable Optimizer::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  OptimizerStatePtr state = loadOptimizerState(context);
  if (!state)
    state = createOptimizerState(context);
  context.enterScope(T("Optimizing - ") + toString());
  jassert(inputs[0].getObjectAndCast<OptimizationProblem>(context));
  OptimizationProblemPtr problem = inputs[0].getObjectAndCast<OptimizationProblem>(context);
  OptimizerStatePtr res = optimize(context, state, problem);
  context.resultCallback(T("bestSolution"), state->getBestSolution());
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
{
  if (!state.isTable(1))
    return Function::__call(state);

  Variable objective = state.getTableVariable(1, "objective");
  Variable initialGuess = state.getTableVariable(1, "initialGuess");
  Variable sampler = state.getTableVariable(1, "sampler");
  Variable validation = state.getTableVariable(1, "validation");
  OptimizationProblemPtr problem = new OptimizationProblem(
    objective.getObjectAndCast<Function>(), initialGuess,
    sampler.getObjectAndCast<Sampler>(), validation.getObjectAndCast<Function>());
 
  OptimizerStatePtr res = compute(state.getContext(), problem).getObjectAndCast<OptimizerState>();
  if (!res)
    return 0;

  state.pushNumber(res->getBestScore());
  state.pushVariable(res->getBestSolution());
  return 2;
}
