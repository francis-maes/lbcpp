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
# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionContainer.h>
# include "SharkProblems.h"

namespace lbcpp
{

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    //testSingleObjectiveOptimizers(context);
    testBiObjectiveOptimizers(context);
    //testSolutionVectorComponent(context);
    return ObjectPtr();
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
        new CrossEntropySolver(new DiagonalGaussianSampler(), 100, numTrainingSamples)));
      context.resultCallback("level1", evaluateSingleObjectiveOptimizer(context, problems, 
        new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 1, 100, numTrainingSamples)));
      context.resultCallback("level2", evaluateSingleObjectiveOptimizer(context, problems, 
        new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 2, 100, numTrainingSamples)));
      context.resultCallback("level3", evaluateSingleObjectiveOptimizer(context, problems, 
        new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 3, 100, numTrainingSamples)));
      context.leaveScope();
    }
#endif // 0

    for (size_t i = 0; i < problems.size(); ++i)
    {
      ProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithSingleObjectiveOptimizer(context, problem, randomSolver(uniformContinuousSampler(), numEvaluations));
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), 100, 50, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), 100, 50, numEvaluations / 100, true));

      /*
      double explorationCoefficient = 5.0;
      IterativeSolverPtr baseOptimizer = crossEntropySolver(diagonalGaussianSampler(), 100, 50, 0, true);
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 2, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 5, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 10, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 20, explorationCoefficient, numEvaluations / 100));
      solveWithSingleObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 50, explorationCoefficient, numEvaluations / 100));*/
/*

      for (double r = -5.5; r <= -0.5; r += 1.0)
        solveWithSingleObjectiveOptimizer(context, problem, new NRPASolver(new DiagonalGaussianSampler(pow(10, r)), 1, numEvaluations));

      for (double r = -5.5; r <= -0.5; r += 1.0)
        solveWithSingleObjectiveOptimizer(context, problem, new NRPASolver(new DiagonalGaussianSampler(pow(10, r)), 2, (size_t)(sqrt((double)numEvaluations))));

      double r = 0.2;
      for (size_t l = 1; l <= 5; ++l)
        solveWithSingleObjectiveOptimizer(context, problem, new NRPASolver(new DiagonalGaussianSampler(r), l, 10));
*/
      context.leaveScope(); 
    }
  }

  double evaluateSingleObjectiveOptimizer(ExecutionContext& context, const std::vector<ProblemPtr>& problems, SolverPtr optimizer, size_t numRuns = 5)
  {
    ScalarVariableMeanAndVariance stats;
    for (size_t i = 0; i < numRuns; ++i)
      for (size_t j = 0; j < problems.size(); ++j)
      {
        FitnessPtr bestFitness;
        SolverCallbackPtr callback = compositeSolverCallback(storeBestFitnessSolverCallback(bestFitness), maxEvaluationsSolverCallback(numEvaluations));
        optimizer->solve(context, problems[j], callback);
        jassert(bestFitness);
        stats.push(bestFitness->getValue(0));
      }
    return stats.getMean();
  }

  double solveWithSingleObjectiveOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer)
  {
    context.enterScope(optimizer->toShortString());

    DenseDoubleVectorPtr cpuTimes = new DenseDoubleVector(0, 0.0);
    DenseDoubleVectorPtr scores = new DenseDoubleVector(0, 0.0);
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront();
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      singleObjectiveEvaluatorSolverCallback(evaluationPeriod, cpuTimes, scores),
      maxEvaluationsSolverCallback(numEvaluations));

    optimizer->setVerbosity((SolverVerbosity)verbosity);
    optimizer->solve(context, problem, callback);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("front", front);

    if (verbosity >= 1)
    {
      context.enterScope("curve");

      for (size_t i = 0; i < scores->getNumValues(); ++i)
      {
        size_t numEvaluations = i * evaluationPeriod;
        context.enterScope(String((int)numEvaluations));
        context.resultCallback("numEvaluations", numEvaluations);
        context.resultCallback("score", scores->getValue(i));
        context.resultCallback("cpuTime", cpuTimes->getValue(i));
        context.leaveScope();
      }
      context.leaveScope();
    }

    jassert(!front->isEmpty());
    double score = front->getFitness(0)->getValue(0);
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
      solveWithMultiObjectiveOptimizer(context, problem, randomSolver(uniformContinuousSampler(), numEvaluations));
      solveWithMultiObjectiveOptimizer(context, problem, nsga2moOptimizer(100, numEvaluations / 100));
      //solveWithMultiObjectiveOptimizer(context, problem, new CMAESMOOptimizer(100, 100, numEvaluations / 100));

      //solveWithMultiObjectiveOptimizer(context, problem, new CrossEntropySolver(diagonalGaussianSampler(), 100, 50, numEvaluations / 100, false));
      solveWithMultiObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), 100, 50, numEvaluations / 100, true));
      /*
      double explorationCoefficient = 5.0;
      IterativeSolverPtr baseOptimizer = crossEntropySolver(diagonalGaussianSampler(), 100, 50, 0, true);
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 2, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 5, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 10, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 20, explorationCoefficient, numEvaluations / 100));
      solveWithMultiObjectiveOptimizer(context, problem, new MABMetaSolver(baseOptimizer, 50, explorationCoefficient, numEvaluations / 100));*/

      
      /*solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 0, 100, 50, numEvaluations / 100, false));
      solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 0, 100, 50, numEvaluations / 100, true));

      solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 1, 50, 25, 2, false));
      solveWithMultiObjectiveOptimizer(context, problem, new NestedCrossEntropySolver(new DiagonalGaussianSampler(), 1, 50, 25, 2, true));*/
      
      context.leaveScope();
    }
  }

  void solveWithMultiObjectiveOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer)
  {
    context.enterScope(optimizer->toShortString());

    DenseDoubleVectorPtr cpuTimes = new DenseDoubleVector(0, 0.0);
    DenseDoubleVectorPtr hyperVolumes = new DenseDoubleVector(0, 0.0);
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront();
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      hyperVolumeEvaluatorSolverCallback(evaluationPeriod, cpuTimes, hyperVolumes),
      maxEvaluationsSolverCallback(numEvaluations));


    optimizer->setVerbosity((SolverVerbosity)verbosity);
    optimizer->solve(context, problem, callback);
    context.resultCallback("optimizer", optimizer);
    //context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    if (verbosity >= 1)
    {
      context.enterScope("curve");

      for (size_t i = 0; i < hyperVolumes->getNumValues(); ++i)
      {
        size_t numEvaluations = i * evaluationPeriod;
        context.enterScope(String((int)numEvaluations));
        context.resultCallback("numEvaluations", numEvaluations);
        context.resultCallback("hyperVolume", hyperVolumes->getValue(i));
        context.resultCallback("cpuTime", cpuTimes->getValue(i));
        context.leaveScope();
      }
      context.leaveScope();
    }

    context.leaveScope(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }

  void testSolutionVectorComponent(ExecutionContext& context)
  {
    ProblemPtr problem = new ZDT1MOProblem();
    SamplerPtr sampler = uniformContinuousSampler();
    sampler->initialize(context, problem->getDomain());

    SolutionVectorPtr solutions = new SolutionVector(problem->getFitnessLimits());
    for (size_t i = 0; i < 100; ++i)
    {
      ObjectPtr solution = sampler->sample(context);
      FitnessPtr fitness = problem->evaluate(context, solution);
      solutions->insertSolution(solution, fitness);
    }
    ParetoFrontPtr front = solutions->getParetoFront();
    context.resultCallback("solutions", solutions);
    context.resultCallback("front", front);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
