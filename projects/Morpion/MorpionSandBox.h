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
# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/SolutionContainer.h>
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
    
    SearchActionCodeGeneratorPtr codeGenerator; // FIXME

    testOptimizer(context, randomSolver(logLinearActionCodeSearchSampler(codeGenerator), numEvaluations), problem);
    for (int r = -8; r <= 0; ++r)
    {
      double regularizer = pow(10.0, (double)r);
      testOptimizer(context, crossEntropySolver(logLinearActionCodeSearchSampler(codeGenerator, regularizer), 100, 30, numEvaluations / 100, false), problem);
      testOptimizer(context, crossEntropySolver(logLinearActionCodeSearchSampler(codeGenerator, regularizer), 100, 30, numEvaluations / 100, true), problem);
    }
    testOptimizer(context, nrpaSolver(logLinearActionCodeSearchSampler(codeGenerator), 1, 100), problem);
    testOptimizer(context, nrpaSolver(logLinearActionCodeSearchSampler(codeGenerator), 2, 100), problem);
    testOptimizer(context, nrpaSolver(logLinearActionCodeSearchSampler(codeGenerator), 3, 100), problem);

    return true;
  }

protected:
  friend class MorpionSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;

  void testOptimizer(ExecutionContext& context, SolverPtr optimizer, ProblemPtr problem)
  {
    context.enterScope(optimizer->toShortString());

    MaxIterationsDecoratorProblemPtr decorator(new MaxIterationsDecoratorProblem(problem, numEvaluations));

    ParetoFrontPtr pareto = optimizer->optimize(context, decorator, ObjectPtr(), (Solver::Verbosity)verbosity);
    context.resultCallback("pareto", pareto);

    context.leaveScope(pareto->getFitness(0)->getValue(0));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MORPION_SANDBOX_H_
