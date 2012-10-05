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
    ProblemPtr problem = new MorpionProblem(5, true);
    SamplerPtr sampler = logLinearActionCodeSearchSampler();
    OptimizerPtr optimizer = randomOptimizer(sampler, numEvaluations);
    ParetoFrontPtr pareto = optimizer->optimize(context, problem, (Optimizer::Verbosity)verbosity);
    context.resultCallback("pareto", pareto);
    return true;
  }

protected:
  friend class MorpionSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MORPION_SANDBOX_H_
