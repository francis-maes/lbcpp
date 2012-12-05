/*-----------------------------------------.---------------------------------.
| Filename: MOOSandBox.h                   | Multi Objective Optimization    |
| Author  : Francis Maes                   | SandBox                         |
| Started : 11/09/2012 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SANDBOX_H_
# define LBCPP_MOO_SANDBOX_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/RandomVariable.h>
# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>

# include <ml/SplittingCriterion.h>
# include <ml/ExpressionSampler.h>

# include "SharkProblems.h"
# include "FQIBasedSolver.h"

namespace lbcpp
{

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    testSingleObjectiveOptimizers(context);
    //testBiObjectiveOptimizers(context);
    //testSolutionVectorComponent(context);
    return ObjectPtr();
  }

  SolverPtr createRegressionExtraTreeLearner()
  {
    SamplerPtr expressionVectorSampler = scalarExpressionVectorSampler();
    SolverPtr conditionLearner = randomSplitConditionLearner(expressionVectorSampler);
    //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
    SolverPtr learner = treeLearner(stddevReductionSplittingCriterion(), conditionLearner); 
    //learner->setVerbosity((SolverVerbosity)verbosity);
    learner = simpleEnsembleLearner(learner, 10);
    learner->setVerbosity(verbosityDetailed);
    return learner;
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
    problems.push_back(new SphereProblem(6));
/*    problems.push_back(new AckleyProblem(6));
    problems.push_back(new GriewangkProblem(6));
    problems.push_back(new RastriginProblem(6));
    problems.push_back(new RosenbrockProblem(6));
    problems.push_back(new RosenbrockRotatedProblem(6));*/

#if 0
    for (size_t numTrainingSamples = 10; numTrainingSamples < 100; numTrainingSamples += 5)
    {
      context.enterScope(string((int)numTrainingSamples));
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
      const size_t populationSize = 20;
      
      ProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      solveWithSingleObjectiveOptimizer(context, problem, randomSolver(uniformScalarVectorSampler(), numEvaluations));
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize));
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize, true));
      
      SolverPtr fqiBSolver = new ScalarVectorFQIBasedSolver(populationSize, numEvaluations / populationSize);
      solveWithSingleObjectiveOptimizer(context, problem, fqiBSolver);
      
      /*SolverPtr ceSolver = crossEntropySolver(diagonalGaussianSampler(), 100, 30, 10);
      ceSolver->setVerbosity((SolverVerbosity)verbosity);
      SolverPtr sbSolver = surrogateBasedSolver(uniformScalarVectorSampler(), 20, createRegressionExtraTreeLearner(), ceSolver, numEvaluations);
      solveWithSingleObjectiveOptimizer(context, problem, sbSolver);*/

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

    DVectorPtr cpuTimes = new DVector(0, 0.0);
    DVectorPtr scores = new DVector(0, 0.0);
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
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

      for (size_t i = 0; i < scores->getNumElements(); ++i)
      {
        size_t numEvaluations = i * evaluationPeriod;
        context.enterScope(string((int)numEvaluations));
        context.resultCallback("numEvaluations", numEvaluations);
        context.resultCallback("score", scores->get(i));
        context.resultCallback("cpuTime", cpuTimes->get(i));
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
      solveWithMultiObjectiveOptimizer(context, problem, randomSolver(uniformScalarVectorSampler(), numEvaluations));
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

    DVectorPtr cpuTimes = new DVector();
    DVectorPtr hyperVolumes = new DVector();
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
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

      for (size_t i = 0; i < hyperVolumes->getNumElements(); ++i)
      {
        size_t numEvaluations = i * evaluationPeriod;
        context.enterScope(string((int)numEvaluations));
        context.resultCallback("numEvaluations", numEvaluations);
        context.resultCallback("hyperVolume", hyperVolumes->get(i));
        context.resultCallback("cpuTime", cpuTimes->get(i));
        context.leaveScope();
      }
      context.leaveScope();
    }

    context.leaveScope(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }

  void testSolutionVectorComponent(ExecutionContext& context)
  {
    ProblemPtr problem = new ZDT1MOProblem();
    SamplerPtr sampler = uniformScalarVectorSampler();
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
