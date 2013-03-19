/*-----------------------------------------.---------------------------------.
| Filename: MOOSandBox.h                   | Multi Objective Optimization    |
| Author  : Francis Maes                   | SandBox                         |
| Started : 11/09/2012 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_SANDBOX_H_
# define MOO_SANDBOX_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/RandomVariable.h>
# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>

# include <ml/SplittingCriterion.h>
# include <ml/SelectionCriterion.h>
# include <ml/ExpressionSampler.h>

# include "ExtraTreeIncrementalLearner.h"
# include "SharkProblems.h"
# include "FQIBasedSolver.h"

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numDimensions(6), numEvaluations(1000), verbosity(1) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
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

  size_t numDimensions;
  size_t numEvaluations;
  size_t verbosity;

  /*
  ** Single Objective
  */
  void testSingleObjectiveOptimizers(ExecutionContext& context)
  {
    const size_t numTrees = 100;
    const size_t K = (size_t)(0.5 + sqrt((double)numDimensions));

    std::vector<ProblemPtr> problems;
    problems.push_back(new SphereProblem(6));
    problems.push_back(new AckleyProblem(6));
    problems.push_back(new GriewangkProblem(6));
    problems.push_back(new RastriginProblem(6));
    problems.push_back(new RosenbrockProblem(6));
    problems.push_back(new RosenbrockRotatedProblem(6));

    for (size_t i = 0; i < problems.size(); ++i)
    {
      const size_t populationSize = numDimensions * 10;
      const size_t numBests = populationSize / 3;
      
      ProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      FitnessPtr bestFitness;

      /*
      ** Baselines
      */
      solveWithSingleObjectiveOptimizer(context, problem, randomSolver(uniformScalarVectorSampler(), numEvaluations), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, numBests, numEvaluations / populationSize), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, numBests, numEvaluations / populationSize, true), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, cmaessoOptimizer(numEvaluations), bestFitness);

      /*
      ** Common to surrogate based solvers
      */
      SamplerPtr sbsInitialSampler = latinHypercubeVectorSampler(60);
      SolverPtr sbsInnerSolver = crossEntropySolver(diagonalGaussianSampler(), populationSize, numBests, 5, true);
      sbsInnerSolver->setVerbosity((SolverVerbosity)verbosity);
      VariableEncoderPtr sbsVariableEncoder = scalarVectorVariableEncoder();
      SelectionCriterionPtr sbsSelectionCriterion = expectedImprovementSelectionCriterion(bestFitness);

      /*
      ** Incremental surroggate based solver with extremely randomized trees
      */
      IncrementalLearnerPtr xtIncrementalLearner = new EnsembleIncrementalLearner(new PureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      SolverPtr incrementalXTBasedSolver = incrementalSurrogateBasedSolver(sbsInitialSampler, xtIncrementalLearner, sbsInnerSolver, sbsVariableEncoder, sbsSelectionCriterion, numEvaluations);
      solveWithSingleObjectiveOptimizer(context, problem, incrementalXTBasedSolver, bestFitness);

      /*
      ** Batch surroggate based solver with extremely randomized trees
      */
      SplittingCriterionPtr splittingCriterion = stddevReductionSplittingCriterion();
      SamplerPtr sampler = subsetVectorSampler(scalarExpressionVectorSampler(), K);
      SolverPtr xtBatchLearner = simpleEnsembleLearner(treeLearner(splittingCriterion, randomSplitConditionLearner(sampler)), numTrees);
      xtBatchLearner->setVerbosity((SolverVerbosity)verbosity);
      SolverPtr batchXTBasedSolver = batchSurrogateBasedSolver(sbsInitialSampler, xtBatchLearner, sbsInnerSolver, sbsVariableEncoder, sbsSelectionCriterion, numEvaluations);
      solveWithSingleObjectiveOptimizer(context, problem, batchXTBasedSolver, bestFitness);

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

  double solveWithSingleObjectiveOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer, FitnessPtr& bestFitness)
  {
    context.enterScope(optimizer->toShortString());

    DVectorPtr cpuTimes = new DVector(0, 0.0);
    DVectorPtr scores = new DVector(0, 0.0);
    IVectorPtr evaluations = new IVector(0, 0);
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    bestFitness = FitnessPtr();
    SolverCallbackPtr callback = compositeSolverCallback(
      storeBestFitnessSolverCallback(bestFitness),
      evaluationPeriodEvaluatorSolverCallback(singleObjectiveSolverEvaluator(bestFitness), evaluations, cpuTimes, scores, evaluationPeriod),
      maxEvaluationsSolverCallback(numEvaluations));

    optimizer->setVerbosity((SolverVerbosity)verbosity);
    optimizer->solve(context, problem, callback);
    context.resultCallback("optimizer", optimizer);
    jassert(bestFitness);
    context.resultCallback("bestFitness", bestFitness);

    if (verbosity >= 1)
    {
      context.enterScope("curve");

      for (size_t i = 0; i < scores->getNumElements(); ++i)
      {
        context.enterScope(string(evaluations->get(i)));
        context.resultCallback("numEvaluations", evaluations->get(i));
        context.resultCallback("score", scores->get(i));
        context.resultCallback("cpuTime", cpuTimes->get(i));
        context.leaveScope();
      }
      context.leaveScope();
    }

    double score = bestFitness->getValue(0);
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
      solveWithMultiObjectiveOptimizer(context, problem, cmaesmoOptimizer(100, 100, numEvaluations / 100));

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
    IVectorPtr evaluations = new IVector();
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    SolverCallbackPtr callback = compositeSolverCallback(
      fillParetoFrontSolverCallback(front),
      evaluationPeriodEvaluatorSolverCallback(hyperVolumeSolverEvaluator(front), evaluations, cpuTimes, hyperVolumes, evaluationPeriod),
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
        context.enterScope(string(evaluations->get(i)));
        context.resultCallback("numEvaluations", evaluations->get(i));
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

#endif // !MOO_SANDBOX_H_
