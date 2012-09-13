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
# include "NestedCrossEntropyOptimizer.h"
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
    //testSingleObjectiveOptimizers(context);
    testBiObjectiveOptimizers(context);
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

#if 0
    for (size_t numTrainingSamples = 10; numTrainingSamples < 100; numTrainingSamples += 5)
    {
      context.enterScope(String((int)numTrainingSamples));
      context.resultCallback("numTrainingSamples", numTrainingSamples);
      context.resultCallback("CE", evaluateSingleObjectiveOptimizer(context, problems,
        new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, numTrainingSamples)));
      context.resultCallback("level1", evaluateSingleObjectiveOptimizer(context, problems, 
        new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 1, 100, numTrainingSamples)));
      context.resultCallback("level2", evaluateSingleObjectiveOptimizer(context, problems, 
        new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 2, 100, numTrainingSamples)));
      context.resultCallback("level3", evaluateSingleObjectiveOptimizer(context, problems, 
        new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 3, 100, numTrainingSamples)));
      context.leaveScope();
    }
#endif // 0

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithSingleObjectiveOptimizer(context, problem, new RandomOptimizer(new UniformContinuousSampler(), numEvaluations));
      solveWithSingleObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(new DiagonalGaussianSampler(), 100, 30, numEvaluations / 100));
      context.leaveScope();
    }
  }

  double evaluateSingleObjectiveOptimizer(ExecutionContext& context, const std::vector<MOOProblemPtr>& problems, MOOOptimizerPtr optimizer, size_t numRuns = 5)
  {
    ScalarVariableMeanAndVariance stats;
    for (size_t i = 0; i < numRuns; ++i)
      for (size_t j = 0; j < problems.size(); ++j)
      {
        MaxIterationsDecoratorProblemPtr  decorator(new MaxIterationsDecoratorProblem(problems[j], numEvaluations));

        MOOParetoFrontPtr front = optimizer->optimize(context, decorator);
        jassert(front->getMap().size() == 1);
        stats.push(front->getMap().begin()->first->getValue(0));
      }
    return stats.getMean();
  }

  double solveWithSingleObjectiveOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
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
    return score;
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
      solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyMOOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, true, true));

      /*solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyMOOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, false, false));
      solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyMOOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, false, true));
      solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyMOOptimizer(new DiagonalGaussianSampler(), 100, 50, numEvaluations / 100, true, false));*/
      //solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(100, 100, numEvaluations / 100));
      //solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(5, 100, numEvaluations / 100));
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
    context.resultCallback("frontNumFitnesses", front->getMap().size());
    context.resultCallback("frontSize", front->getNumElements());

    /*std::vector<MOOParetoFrontPtr> subFronts = front->nonDominatedSort();
    for (size_t i = 0; i < subFronts.size(); ++i)
      context.resultCallback("subFront" + String((int)i), subFronts[i]);*/

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
