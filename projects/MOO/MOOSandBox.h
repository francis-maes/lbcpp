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

# include "SharkProblems.h"
# include "FQIBasedSolver.h"

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numDimensions(6), numObjectives(2), numEvaluations(1000), numRuns(1), verbosity(1), useDefaults(false), paretoFrontDir(""), problemIdx(0) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    //testSingleObjectiveOptimizers(context);
    testBiObjectiveOptimizers(context);
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
  size_t numObjectives;
  size_t numEvaluations;
  size_t numRuns;
  size_t verbosity;
  string paretoFrontDir;
  size_t problemIdx;

  bool useDefaults;

  typedef std::pair<double, SolverPtr> SolverResult;

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
      solveWithSingleObjectiveOptimizer(context, problem, randomSolver(uniformSampler(), numEvaluations), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, numBests, numEvaluations / populationSize), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, numBests, numEvaluations / populationSize, true), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, cmaessoOptimizer(numEvaluations), bestFitness);
      solveWithSingleObjectiveOptimizer(context, problem, nsga2moOptimizer(), bestFitness);

      /*
      ** Common to surrogate based solvers
      */
      /*SamplerPtr sbsInitialSampler = latinHypercubeVectorSampler(60);
      SolverPtr sbsInnerSolver = crossEntropySolver(diagonalGaussianSampler(), populationSize, numBests, 5, true);
      sbsInnerSolver->setVerbosity((SolverVerbosity)verbosity);
      VariableEncoderPtr sbsVariableEncoder = scalarVectorVariableEncoder();
      SelectionCriterionPtr sbsSelectionCriterion = expectedImprovementSelectionCriterion(bestFitness);*/

      /*
      ** Incremental surroggate based solver with extremely randomized trees
      */
      /*IncrementalLearnerPtr xtIncrementalLearner = new EnsembleIncrementalLearner(new PureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      SolverPtr incrementalXTBasedSolver = incrementalSurrogateBasedSolver(sbsInitialSampler, xtIncrementalLearner, sbsInnerSolver, sbsVariableEncoder, sbsSelectionCriterion, numEvaluations);
      solveWithSingleObjectiveOptimizer(context, problem, incrementalXTBasedSolver, bestFitness);*/

      /*
      ** Batch surroggate based solver with extremely randomized trees
      */
      /*SplittingCriterionPtr splittingCriterion = stddevReductionSplittingCriterion();
      SamplerPtr sampler = subsetVectorSampler(scalarExpressionVectorSampler(), K);
      SolverPtr xtBatchLearner = simpleEnsembleLearner(treeLearner(splittingCriterion, randomSplitConditionLearner(sampler)), numTrees);
      xtBatchLearner->setVerbosity((SolverVerbosity)verbosity);
      SolverPtr batchXTBasedSolver = batchSurrogateBasedSolver(sbsInitialSampler, xtBatchLearner, sbsInnerSolver, sbsVariableEncoder, sbsSelectionCriterion, numEvaluations);
      solveWithSingleObjectiveOptimizer(context, problem, batchXTBasedSolver, bestFitness);*/

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
    /*
    problems.push_back(new ZDT1MOProblem(numDimensions));
    problems.push_back(new ZDT2MOProblem(numDimensions));
    problems.push_back(new ZDT3MOProblem(numDimensions));
    problems.push_back(new ZDT4MOProblem(numDimensions));
    problems.push_back(new ZDT6MOProblem(numDimensions));
    problems.push_back(new LZ06_F1MOProblem(numDimensions));
    */
    if (useDefaults)
    {
      problems.push_back(new DTLZ1MOProblem());
      problems.push_back(new DTLZ2MOProblem());
      problems.push_back(new DTLZ3MOProblem());
      problems.push_back(new DTLZ4MOProblem());
      problems.push_back(new DTLZ5MOProblem());
      problems.push_back(new DTLZ6MOProblem());
      problems.push_back(new DTLZ7MOProblem());
    }
    else
    {
      problems.push_back(new DTLZ1MOProblem(numDimensions, numObjectives));
      problems.push_back(new DTLZ2MOProblem(numDimensions, numObjectives));
      problems.push_back(new DTLZ3MOProblem(numDimensions, numObjectives));
      problems.push_back(new DTLZ4MOProblem(numDimensions, numObjectives));
      problems.push_back(new DTLZ5MOProblem(numDimensions, numObjectives));
      problems.push_back(new DTLZ6MOProblem(numDimensions, numObjectives));
      problems.push_back(new DTLZ7MOProblem(numDimensions, numObjectives));
    }
    
    
      ProblemPtr problem = problems[problemIdx];
      string path = paretoFrontDir + T("/DTLZ") + string(problemIdx+1) + T(".2D.pf");
      ParetoFrontPtr referenceFront = new ParetoFront(problem->getFitnessLimits(), path);
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      context.resultCallback("Reference front", referenceFront);
      size_t populationSize = 100;
      std::vector<SolverResult> results = std::vector<SolverResult>();
      size_t numSolvers = 7;
      size_t currentSolver = 0;
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, randomSolver(uniformSampler(), numEvaluations), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, nsga2moOptimizer(populationSize, numEvaluations / populationSize), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, cmaesmoOptimizer(populationSize, populationSize, numEvaluations / populationSize), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 4, numEvaluations / populationSize, true), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, smpsoOptimizer(populationSize, populationSize, numEvaluations / populationSize, samplerToVectorSampler(uniformSampler(), 100)), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, omopsoOptimizer(populationSize, populationSize, numEvaluations / populationSize, samplerToVectorSampler(uniformSampler(), 100)), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, abYSSOptimizer(populationSize, populationSize, populationSize / 2, populationSize / 2, numEvaluations / populationSize), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      
      std::vector<SolverResult>::iterator best = results.begin();
      for (std::vector<SolverResult>::iterator it = results.begin(); it != results.end(); ++it)
        if (it->first > best->first) best = it;
      context.leaveScope(best->second);
    
  }

  SolverResult solveWithMultiObjectiveOptimizer(ExecutionContext& context, ProblemPtr problem, SolverPtr optimizer, ParetoFrontPtr referenceFront)
  {
    context.enterScope(optimizer->toShortString());

    ScalarVariableMeanAndVariancePtr hvs = new ScalarVariableMeanAndVariance();
    context.progressCallback(new ProgressionState(0, numRuns, "Runs"));
    std::vector<ScalarVariableMeanAndVariancePtr>* values = new std::vector<ScalarVariableMeanAndVariancePtr>();
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    SolverCallbackPtr aggregator = aggregatorEvaluatorSolverCallback(hyperVolumeSolverEvaluator(front), values);
    for (size_t i = 0; i < numRuns; ++i)
    {
      context.enterScope("Run " + string((int) i));
      DVectorPtr cpuTimes = new DVector();
      DVectorPtr hyperVolumes = new DVector();
      IVectorPtr evaluations = new IVector();
      size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
      SolverCallbackPtr callback = compositeSolverCallback(
        fillParetoFrontSolverCallback(front),
        //evaluationPeriodEvaluatorSolverCallback(hyperVolumeSolverEvaluator(front), evaluations, cpuTimes, hyperVolumes, evaluationPeriod),
        aggregator,
        maxEvaluationsSolverCallback(numEvaluations));


      optimizer->setVerbosity((SolverVerbosity)verbosity);
      optimizer->solve(context, problem, callback);
      context.resultCallback("optimizer", optimizer);
      context.resultCallback("Additive epsilon", front->computeAdditiveEpsilonIndicator(referenceFront));
      context.resultCallback("Multiplicative epsilon", front->computeMultiplicativeEpsilonIndicator(referenceFront));
      //context.resultCallback("numEvaluations", decorator->getNumEvaluations());

      /*
      if (verbosity >= verbosityDetailed)
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
      }*/

      double hv = front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
      front->clear();
      context.leaveScope(hv);
      hvs->push(hv);
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }
    context.enterScope("curve");
    for (size_t i = 0; i < values->size(); ++i)
    {
      context.enterScope(string(i));
      context.resultCallback("NumEvaluations", i);
      context.resultCallback("Hypervolume", values->at(i));
      context.leaveScope();
    }
    context.leaveScope();
    delete values;
    context.leaveScope(hvs);
    return SolverResult(hvs->getMean(), optimizer);
  }

  void testSolutionVectorComponent(ExecutionContext& context)
  {
    ProblemPtr problem = new ZDT1MOProblem();
    SamplerPtr sampler = uniformSampler();
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
