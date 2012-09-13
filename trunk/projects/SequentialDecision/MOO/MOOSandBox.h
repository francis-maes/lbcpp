/*-----------------------------------------.---------------------------------.
| Filename: MOOSandBox.h                   | Multi Objective Optimization    |
| Author  : Francis Maes                   | SandBox                         |
| Started : 11/09/2012 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SANDBOX_H_
# define LBCPP_MOO_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "SharkMOOptimizers.h"
# include "RandomOptimizer.h"
# include "SharkMOProblems.h"
# include "DecoratorProblems.h"
# include "CrossEntropyOptimizer.h"
# include "UniformContinuousSampler.h"
# include "DiagonalGaussianContinuousSampler.h"

namespace lbcpp
{

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numEvaluations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    testSingleObjectiveOptimizers(context);
    //testBiObjectiveOptimizers(context);
    return true;
  }

protected:
  friend class MOOSandBoxClass;

  size_t numEvaluations;


  void testSingleObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    
  }

  void testBiObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    problems.push_back(new ZDT1MOProblem());
    problems.push_back(new ZDT2MOProblem());
    problems.push_back(new ZDT3MOProblem());
    problems.push_back(new ZDT4MOProblem());
    problems.push_back(new ZDT6MOProblem());

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());

      context.resultCallback("problem", problem);
      solveWithMultiObjectiveOptimizer(context, problem, new RandomOptimizer(new UniformContinuousSampler()));
      solveWithMultiObjectiveOptimizer(context, problem, new NSGA2MOOptimizer(100));
      solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(100, 100));
      solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(5, 100));
      context.leaveScope();
    }
  }

  void solveWithMultiObjectiveOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorProblemPtr decorator(new HyperVolumeEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr paretoFront = optimizer->optimize(context, decorator);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("paretoFront", paretoFront);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    //context.enterScope("HyperVolume Curve");
    std::vector<double> hyperVolumes = decorator->getHyperVolumes();
    std::vector<double> cpuTimes = decorator->getCpuTimes();

    for (size_t i = 0; i < hyperVolumes.size(); ++i)
    {
      size_t numEvaluations = i * decorator->getHyperVolumeComputationPeriod();
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("hyperVolume", hyperVolumes[i]);
      context.resultCallback("cpuTime", cpuTimes[i]);
      context.leaveScope();
    }
    //context.leaveScope();

    context.leaveScope(paretoFront->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
