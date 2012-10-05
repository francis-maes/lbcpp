/*-----------------------------------------.---------------------------------.
| Filename: MorpionSandBox.h               | Morpion Solitaire SandBox       |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2012 12:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MORPION_SANDBOX_H_
# define LBCPP_MORPION_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/SolutionSet.h>
# include "MorpionProblem.h"

namespace lbcpp
{

class MorpionSandBox : public WorkUnit
{
public:
  MorpionSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual Variable run(ExecutionContext& context)
  {
    ProblemPtr problem = new MorpionProblem(4, true);
    
    /*testOptimizer(context, randomOptimizer(logLinearActionCodeSearchSampler(), numEvaluations), problem);
    testOptimizer(context, crossEntropyOptimizer(logLinearActionCodeSearchSampler(), 100, 30, numEvaluations / 100, false), problem);
    testOptimizer(context, crossEntropyOptimizer(logLinearActionCodeSearchSampler(), 100, 30, numEvaluations / 100, true), problem);
    testOptimizer(context, nrpaOptimizer(logLinearActionCodeSearchSampler(), 1, 100), problem);
    testOptimizer(context, nrpaOptimizer(logLinearActionCodeSearchSampler(), 2, 100), problem);*/
    testOptimizer(context, nrpaOptimizer(logLinearActionCodeSearchSampler(), 3, 100), problem);

    // todo: cross-entropy with logLinearActionCodeSearchSampler

    return true;
  }

protected:
  friend class MorpionSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;

  void testOptimizer(ExecutionContext& context, OptimizerPtr optimizer, ProblemPtr problem)
  {
    context.enterScope(optimizer->toShortString());

    MaxIterationsDecoratorProblemPtr decorator(new MaxIterationsDecoratorProblem(problem, numEvaluations));

    ParetoFrontPtr pareto = optimizer->optimize(context, decorator, (Optimizer::Verbosity)verbosity);
    context.resultCallback("pareto", pareto);

    context.leaveScope(pareto->getSolution(0)->getFitness()->getValue(0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MORPION_SANDBOX_H_
