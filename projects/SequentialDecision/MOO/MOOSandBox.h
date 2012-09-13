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
# include "SharkProblems.h"
# include "DecoratorProblems.h"
# include "NRPAOptimizer.h"
# include "CrossEntropyOptimizer.h"
# include "UniformContinuousSampler.h"
# include "DiagonalGaussianSampler.h"

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

  /*
  ** Single Objective
  */
  void testSingleObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    problems.push_back(new AckleyProblem());
    problems.push_back(new GriewangkProblem());
    problems.push_back(new RastriginProblem());
    problems.push_back(new RosenbrockProblem());
    problems.push_back(new RosenbrockRotatedProblem());

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithSingleObjectiveOptimizer(context, problem, new RandomOptimizer(new UniformContinuousSampler(), numEvaluations));
      solveWithSingleObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, 30, numEvaluations / 100));
      
      for (double r = -1.0; r <= 1.0; r += 0.5)
        for (int l = 1; l <= 3; ++l)
          solveWithSingleObjectiveOptimizer(context, problem, new NRPAOptimizer(new DiagonalGaussianSampler(pow(10.0, r)), l, 100));

      context.leaveScope();
    }
  }

  void solveWithSingleObjectiveOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    SingleObjectiveEvaluatorDecoratorProblemPtr decorator(new SingleObjectiveEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr front = optimizer->optimize(context, decorator);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("front", front);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    std::vector<double> cpuTimes = decorator->getCpuTimes();
    std::vector<double> scores = decorator->getScores();

    for (size_t i = 0; i < scores.size(); ++i)
    {
      size_t numEvaluations = i * decorator->getEvaluationPeriod();
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("score", scores[i]);
      context.resultCallback("cpuTime", cpuTimes[i]);
      context.leaveScope();
    }

    jassert(front->getMap().size() == 1);
    double score = front->getMap().begin()->first->getValue(0);
    context.resultCallback("score", score);
    context.leaveScope(score);
  }

  /*
  ** Multi-objective
  */
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
      solveWithMultiObjectiveOptimizer(context, problem, new RandomOptimizer(new UniformContinuousSampler(), numEvaluations));
      solveWithMultiObjectiveOptimizer(context, problem, new NSGA2MOOptimizer(100, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(100, 100, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(5, 100, numEvaluations / 100));
      context.leaveScope();
    }
  }

  void solveWithMultiObjectiveOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorProblemPtr decorator(new HyperVolumeEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr front = optimizer->optimize(context, decorator);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("front", front);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    std::vector<double> hyperVolumes = decorator->getHyperVolumes();
    std::vector<double> cpuTimes = decorator->getCpuTimes();

    for (size_t i = 0; i < hyperVolumes.size(); ++i)
    {
      size_t numEvaluations = i * decorator->getEvaluationPeriod();
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("hyperVolume", hyperVolumes[i]);
      context.resultCallback("cpuTime", cpuTimes[i]);
      context.leaveScope();
    }

    context.leaveScope(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
