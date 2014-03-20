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
# include "WFGProblems.h"
# include "FQIBasedSolver.h"

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numDimensions(6), numObjectives(2), numEvaluations(1000), numRuns(1), verbosity(1), 
    useDefaults(false), paretoFrontDir(""), problemIdx(0), archiveSize(200) {}

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
  size_t archiveSize;

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
    std::vector<string> pfFiles; // reference pareto front files

    pfFiles.push_back(T("WFG1.2D.pf"));
    pfFiles.push_back(T("WFG2.2D.pf"));
    pfFiles.push_back(T("WFG3.2D.pf"));
    pfFiles.push_back(T("WFG4.2D.pf"));
    pfFiles.push_back(T("WFG5.2D.pf"));
    pfFiles.push_back(T("WFG6.2D.pf"));
    pfFiles.push_back(T("WFG7.2D.pf"));
    pfFiles.push_back(T("WFG8.2D.pf"));
    pfFiles.push_back(T("WFG9.2D.pf"));
    pfFiles.push_back(T("DTLZ1.2D.pf"));
    pfFiles.push_back(T("DTLZ2.2D.pf"));
    pfFiles.push_back(T("DTLZ3.2D.pf"));
    pfFiles.push_back(T("DTLZ4.2D.pf"));
    pfFiles.push_back(T("DTLZ5.2D.pf"));
    pfFiles.push_back(T("DTLZ6.2D.pf"));
    pfFiles.push_back(T("DTLZ7.2D.pf"));

    if (useDefaults)
    {
      problems.push_back(new WFG1Problem());
      problems.push_back(new WFG2Problem());
      problems.push_back(new WFG3Problem());
      problems.push_back(new WFG4Problem());
      problems.push_back(new WFG5Problem());
      problems.push_back(new WFG6Problem());
      problems.push_back(new WFG7Problem());
      problems.push_back(new WFG8Problem());
      problems.push_back(new WFG9Problem());
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
      string path = paretoFrontDir + T("/") + pfFiles[problemIdx];
      ParetoFrontPtr referenceFront = new ParetoFront(problem->getFitnessLimits(), path);
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      if (verbosity >= verbosityDetailed)
        context.resultCallback("Reference front", referenceFront);
      context.resultCallback("Reference front hypervolume", referenceFront->computeHyperVolume());
      size_t populationSize = 100;
      std::vector<SolverResult> results = std::vector<SolverResult>();
      size_t numSolvers = 5;
      size_t currentSolver = 0;
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, nsga2moOptimizer(populationSize, numEvaluations / populationSize), referenceFront));
      /*
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, cmaesmoOptimizer(populationSize, populationSize, numEvaluations / populationSize), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 4, numEvaluations / populationSize, true), referenceFront));
      */
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, smpsoOptimizer(populationSize, populationSize, numEvaluations / populationSize, samplerToVectorSampler(uniformSampler(), 100)), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, omopsoOptimizer(populationSize, populationSize, numEvaluations / populationSize, samplerToVectorSampler(uniformSampler(), 100)), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, abYSSOptimizer(populationSize, populationSize, populationSize / 2, populationSize / 2, numEvaluations / populationSize), referenceFront));
      context.progressCallback(new ProgressionState((size_t) currentSolver++, numSolvers, "Solvers"));
      results.push_back(solveWithMultiObjectiveOptimizer(context, problem, randomSolver(uniformSampler(), numEvaluations), referenceFront));
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
    ParetoFrontPtr front = new CrowdingArchive(archiveSize, problem->getFitnessLimits());
    size_t evaluationPeriod = numEvaluations > 250 ? numEvaluations / 250 : 1;
    
    // aggregator setup
    std::vector<SolverEvaluatorPtr> evaluators;
    evaluators.push_back(hyperVolumeSolverEvaluator(front));
    evaluators.push_back(additiveEpsilonSolverEvaluator(front, referenceFront));
    evaluators.push_back(spreadSolverEvaluator(front));
    std::map<string, std::vector<EvaluationPoint> >* data = new std::map<string, std::vector<EvaluationPoint> >();

    for (size_t i = 0; i < numRuns; ++i)
    {
      if (verbosity >= verbosityProgressAndResult)
        context.enterScope("Run " + string((int) i));
      DVectorPtr cpuTimes = new DVector();
      DVectorPtr hyperVolumes = new DVector();
      IVectorPtr evaluations = new IVector();
      SolverCallbackPtr callback = compositeSolverCallback(
        fillParetoFrontSolverCallback(front),
        aggregatorEvaluatorSolverCallback(evaluators, data, evaluationPeriod),
        maxEvaluationsSolverCallback(numEvaluations));
      optimizer->setVerbosity((SolverVerbosity)verbosity);
      optimizer->solve(context, problem, callback);
      double hv = front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());
      front = new CrowdingArchive(archiveSize, problem->getFitnessLimits());
      if (verbosity >= verbosityProgressAndResult)
        context.leaveScope(hv);
      hvs->push(hv);
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }
    
    context.enterScope("curve");
    for (size_t i = 0; i < data->begin()->second.size(); ++i)
    {
      context.enterScope(string((int)i));
      context.resultCallback("NumEvaluations", data->begin()->second[i].getNumEvaluations());
      for (std::map<string, std::vector<EvaluationPoint> >::iterator it = data->begin(); it != data->end(); ++it)
      {
        context.resultCallback(it->first, it->second[i].getSummary());
        //context.resultCallback(it->first + T(" stddev"), it->second[i].getSummary()->getStandardDeviation());
      }
      context.leaveScope();
    }
    context.leaveScope();
    delete data;
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
