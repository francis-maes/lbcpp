/*-----------------------------------------.---------------------------------.
| Filename: MCOptimizer.h                  | Monte Carlo Optimizer           |
| Author  : Francis Maes                   |                                 |
| Started : 07/04/2012 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_OPTIMIZER_H_
# define LBCPP_LUAPE_MC_OPTIMIZER_H_

# include "MCAlgorithm.h"

namespace lbcpp
{

/*
** Optimizer
*/
class WrapperMCObjective : public MCObjective
{
public:
  WrapperMCObjective(FunctionPtr lbcppObjective, size_t budget)
    : lbcppObjective(lbcppObjective), budget(budget), numEvaluations(0), numCachedEvaluations(0) {}

  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
  {
    LuapeGraphBuilderStatePtr builder = finalState.staticCast<LuapeGraphBuilderState>();
    if (builder->getStackSize() != 1)
      return -DBL_MAX;
    LuapeNodePtr node = builder->getStackElement(0);
    std::map<LuapeNodePtr, double>::iterator it = cache.find(node);
    if (it != cache.end())
    {
      ++numCachedEvaluations;
      return it->second;
    }
    ++numEvaluations;
    double res = lbcppObjective->compute(context, finalState).toDouble();
    cache[node] = res;
    return res;
  }

  virtual bool shouldStop() const
    {return numEvaluations >= budget || numCachedEvaluations >= 100 * budget;}

protected:
  FunctionPtr lbcppObjective;
  size_t budget;
  size_t numEvaluations;
  size_t numCachedEvaluations;
  std::map<LuapeNodePtr, double> cache;
};

class MCOptimizer : public Optimizer
{
public:
  MCOptimizer(MCAlgorithmPtr algorithm, size_t budget)
    : algorithm(algorithm), budget(budget) {}
  MCOptimizer() : budget(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    DecisionProblemStatePtr bestFinalState;
    MCAlgorithmPtr algorithm = new IterateMCAlgorithm(this->algorithm, 0x7FFFFFFF); // repeat base algorithm until budget is exhausted
    double bestScore = algorithm->search(context, new WrapperMCObjective(problem->getObjective(), budget), problem->getInitialState(), NULL, bestFinalState);
    optimizerState->submitSolution(bestFinalState, bestScore);
    return optimizerState;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new OptimizerState(problem);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Optimizer::clone(context, target);
    target.staticCast<MCOptimizer>()->algorithm = algorithm->cloneAndCast<MCAlgorithm>();
  }

protected:
  friend class MCOptimizerClass;

  MCAlgorithmPtr algorithm;
  size_t budget;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_OPTIMIZER_H_
