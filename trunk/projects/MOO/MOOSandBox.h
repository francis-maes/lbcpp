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
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionSet.h>
# include "SharkProblems.h"

namespace lbcpp
{

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual Variable run(ExecutionContext& context)
  {
    testSingleObjectiveOptimizers(context);
    testBiObjectiveOptimizers(context);
    //testSolutionSetComponent(context);
    return true;
  }

protected:
  friend class MOOSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;

  /*
  ** Single Objective
  */
  void testSingleObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<ProblemPtr> problems;
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
      ProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithSingleObjectiveOptimizer(context, problem, randomOptimizer(uniformContinuousSampler(), numEvaluations));
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropyOptimizer(diagonalGaussianSampler(), 100, 50, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropyOptimizer(diagonalGaussianSampler(), 100, 50, numEvaluations / 100, true));

      /*
      double explorationCoefficient = 5.0;
      IterativeOptimizerPtr baseOptimizer = crossEntropyOptimizer(diagonalGaussianSampler(), 100, 50, 0, true);
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 2, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 5, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 10, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 20, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 50, explorationCoefficient, numEvaluations / 100));*/
/*

      for (double r = -5.5; r <= -0.5; r += 1.0)
        solveWithSingleObjectiveOptimizer(context, problem, new NRPAOptimizer(new DiagonalGaussianSampler(pow(10, r)), 1, numEvaluations));

      for (double r = -5.5; r <= -0.5; r += 1.0)
        solveWithSingleObjectiveOptimizer(context, problem, new NRPAOptimizer(new DiagonalGaussianSampler(pow(10, r)), 2, (size_t)(sqrt((double)numEvaluations))));

      double r = 0.2;
      for (size_t l = 1; l <= 5; ++l)
        solveWithSingleObjectiveOptimizer(context, problem, new NRPAOptimizer(new DiagonalGaussianSampler(r), l, 10));
*/
      context.leaveScope(); 
    }
  }

  double evaluateSingleObjectiveOptimizer(ExecutionContext& context, const std::vector<ProblemPtr>& problems, OptimizerPtr optimizer, size_t numRuns = 5)
  {
    ScalarVariableMeanAndVariance stats;
    for (size_t i = 0; i < numRuns; ++i)
      for (size_t j = 0; j < problems.size(); ++j)
      {
        MaxIterationsDecoratorProblemPtr decorator(new MaxIterationsDecoratorProblem(problems[j], numEvaluations));

        ParetoFrontPtr front = optimizer->optimize(context, decorator);
        jassert(!front->isEmpty());
        stats.push(front->getSolution(0)->getFitness()->getValue(0));
      }
    return stats.getMean();
  }

  double solveWithSingleObjectiveOptimizer(ExecutionContext& context, ProblemPtr problem, OptimizerPtr optimizer)
  {
    SingleObjectiveEvaluatorDecoratorProblemPtr decorator(new SingleObjectiveEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    ParetoFrontPtr front = optimizer->optimize(context, decorator, ObjectPtr(), (Optimizer::Verbosity)verbosity);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("front", front);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    if (verbosity >= 1)
    {
      context.enterScope("curve");
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
      context.leaveScope();
    }

    jassert(!front->isEmpty());
    double score = front->getSolution(0)->getFitness()->getValue(0);
    context.resultCallback("score", score);
    context.leaveScope(score);
    return score;
  }

  /*
  ** Multi-objective
  */
  void testBiObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<ProblemPtr> problems;
    problems.push_back(new ZDT1MOProblem());
    problems.push_back(new ZDT2MOProblem());
    problems.push_back(new ZDT3MOProblem());
    problems.push_back(new ZDT4MOProblem());
    problems.push_back(new ZDT6MOProblem());

    for (size_t i = 0; i < problems.size(); ++i)
    {
      ProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithMultiObjectiveOptimizer(context, problem, randomOptimizer(uniformContinuousSampler(), numEvaluations));
      solveWithMultiObjectiveOptimizer(context, problem, nsga2moOptimizer(100, numEvaluations / 100));
      //solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(100, 100, numEvaluations / 100));

      //solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropyOptimizer(diagonalGaussianSampler(), 100, 50, numEvaluations / 100, false));
      solveWithMultiObjectiveOptimizer(context, problem, crossEntropyOptimizer(diagonalGaussianSampler(), 100, 50, numEvaluations / 100, true));
      /*
      double explorationCoefficient = 5.0;
      IterativeOptimizerPtr baseOptimizer = crossEntropyOptimizer(diagonalGaussianSampler(), 100, 50, 0, true);
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 2, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 5, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 10, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 20, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaOptimizer(baseOptimizer, 50, explorationCoefficient, numEvaluations / 100));*/

      
      /*solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 0, 100, 50, numEvaluations / 100, false));
      solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 0, 100, 50, numEvaluations / 100, true));

      solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 1, 50, 25, 2, false));
      solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropyOptimizer(new DiagonalGaussianSampler(), 1, 50, 25, 2, true));*/
      
      context.leaveScope();
    }
  }

  void solveWithMultiObjectiveOptimizer(ExecutionContext& context, ProblemPtr problem, OptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorProblemPtr decorator(new HyperVolumeEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    ParetoFrontPtr front = optimizer->optimize(context, decorator, ObjectPtr(), (Optimizer::Verbosity)verbosity);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    if (verbosity >= 1)
    {
      context.enterScope("curve");
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
      context.leaveScope();
    }

    context.leaveScope(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }

  void testSolutionSetComponent(ExecutionContext& context)
  {
    ProblemPtr problem = new ZDT1MOProblem();
    SamplerPtr sampler = uniformContinuousSampler();
    sampler->initialize(context, problem->getDomain());

    SolutionSetPtr solutions = new SolutionSet(problem->getFitnessLimits());
    for (size_t i = 0; i < 100; ++i)
    {
      ObjectPtr object = sampler->sample(context);
      FitnessPtr fitness = problem->evaluate(context, object);
      solutions->addSolution(object, fitness);
    }
    ParetoFrontPtr front = solutions->getParetoFront();
    context.resultCallback("solutions", solutions);
    context.resultCallback("front", front);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
