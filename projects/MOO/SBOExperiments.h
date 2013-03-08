/*-----------------------------------------.---------------------------------.
 | Filename: SBOExperiments                 | Surrogate-Based Optimization    |
 | Author  : Denny Verbeeck                 | Experimental Evaluation         |
 | Started : 04/03/2013 17:20               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef SBO_EXPERIMENTS_H_
# define SBO_EXPERIMENTS_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/RandomVariable.h>
# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>

# include <ml/SplittingCriterion.h>
# include <ml/SelectionCriterion.h>

# include "SharkProblems.h"

namespace lbcpp
{
  
extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class SBOExperiments : public WorkUnit
{
public:
  SBOExperiments() :  runBaseline(false),
                      numEvaluationsBaseline(1000), 
                      populationSize(20),
                      runRandomForests(false),
                      runXT(false),
                      uniformSampling(false),
                      latinHypercubeSampling(false),
                      numEvaluationsSBO(500), 
                      numTrees(100),
                      optimism(2.0),
                      numDims(6),
                      numRuns(10),
                      verbosity(1),
                      optimizerVerbosity(1),
                      problemIdx(1) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    jassert(problemIdx >= 1 && problemIdx <= 6);
    testSingleObjectiveOptimizers(context);
    //testBiObjectiveOptimizers(context);
    return ObjectPtr();
  }
  
protected:
  friend class SBOExperimentsClass;
  
  // options for baseline algorithms
  bool runBaseline;              /**< Run baseline algorithms                                          */
  size_t numEvaluationsBaseline; /**< Number of evaluations for baseline optimizers                    */
  size_t populationSize;         /**< Population size for baseline optimizers                          */
  
  // options for surrogate-based algorithms
  bool runRandomForests;     /**< Run with random forest surrogate                                 */
  bool runXT;                /**< Run with extremely randomized trees                              */

  bool uniformSampling;          /**< Run with uniform sampling                                        */
  bool latinHypercubeSampling;   /**< Run with latin hypercube sampling                                */

  size_t numEvaluationsSBO;      /**< Number of evaluations for surrogate-based optimizers             */
  size_t numTrees;               /**< Size of forest for Random Forest and extremely randomized trees  */
  
  double optimism;               /**< Level of optimism for optimistic surrogate-based optimizers      */

  // general options
  size_t numDims;                /**< Number of dimensions of the decision space                       */
  size_t numRuns;                /**< Number of runs to average over                                   */
  
  size_t verbosity;
  size_t optimizerVerbosity;
  
  size_t problemIdx;             /**< The problem to run (1-6)                                         */
  
  struct SolverInfo
  {
    string name;
    size_t evaluationPeriod;
    std::vector<double> cpuTimes;
    std::vector<double> scores;
  };
  
  struct SolverSettings
  { 
    SolverSettings() : bestFitness(NULL) {}
    
    SolverPtr solver;
    FitnessPtr* bestFitness;
    string description;
    size_t numEvaluations;
  };
  
  SolverSettings createSettings(SolverPtr solver, size_t numEvaluations, const string& description = string::empty, FitnessPtr* bestFitness = NULL)
  {
    SolverSettings res;
    res.bestFitness = bestFitness;
    res.solver = solver;
    res.numEvaluations = numEvaluations;
    res.description = description.isEmpty() ? solver->toShortString() : description;
    return res;
  }
  
  SolverInfo runSolver(ExecutionContext& context, ProblemPtr problem, SolverSettings solverSettings)
  {
    context.enterScope(solverSettings.description);
    context.resultCallback("solver", solverSettings.solver);
    std::vector<SolverInfo> runInfos(numRuns);
    for (size_t i = 0; i < numRuns; ++i)
    {
      SolverInfo& info = runInfos[i];
      context.enterScope("Run " + string((int)i));
      double res = runSolverOnce(context, problem, solverSettings, info);
      context.leaveScope(res);
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }
    
    SolverInfo res;
    res.name = solverSettings.description;
    res.scores.resize(runInfos[0].scores.size());
    res.evaluationPeriod = runInfos[0].evaluationPeriod;
    double best = DBL_MAX;
    mergeResults(res.scores, best, runInfos, false);
    context.leaveScope(best);
    return res;
  }
  
  double runSolverOnce(ExecutionContext& context, ProblemPtr problem, SolverSettings solverSettings, SolverInfo& info)
  {
    problem->reinitialize(context);

    FitnessPtr defaultBestFitness;
    if (solverSettings.bestFitness)
      *solverSettings.bestFitness = FitnessPtr();
    else
      solverSettings.bestFitness = &defaultBestFitness;
    
    DVectorPtr cpuTimes = new DVector();
    DVectorPtr scores = new DVector();
    size_t evaluationPeriod = 2;
    SolverCallbackPtr callback = compositeSolverCallback(storeBestFitnessSolverCallback(*solverSettings.bestFitness),
                                           singleObjectiveEvaluatorSolverCallback(evaluationPeriod, cpuTimes, scores),
                                           maxEvaluationsSolverCallback(solverSettings.numEvaluations));
    
    solverSettings.solver->setVerbosity((SolverVerbosity)optimizerVerbosity);
    solverSettings.solver->solve(context, problem, callback);
    
    context.resultCallback("optimizer", solverSettings.solver);
    if (verbosity >= verbosityDetailed)
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

    info.cpuTimes = cpuTimes->getNativeVector();
    info.scores = scores->getNativeVector();
    info.evaluationPeriod = evaluationPeriod;
    return (*solverSettings.bestFitness)->toDouble();
  }
  
  void mergeResults(std::vector<double>& res, double& best, const std::vector<SolverInfo>& infos, bool inFunctionOfCpuTime)
  {
    for (size_t i = 0; i < res.size(); ++i)
    {
      ScalarVariableMean mean;
      for (size_t j = 0; j < infos.size(); ++j)
        mean.push(infos[j].scores[i]);
      res[i] = mean.getMean();
      best = (best < res[i] ? best : res[i]);
    }
  }
  
  void displayResults(ExecutionContext& context, const std::vector<SolverInfo>& infos)
  {
    for (size_t i = 0; i < infos[0].scores.size(); ++i)
    {
      size_t numEvaluations = (i + 1) * infos[0].evaluationPeriod;
      context.enterScope(string((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("log(numEvaluations)", log10((double)numEvaluations));
      for (size_t j = 0; j < infos.size(); ++j)
        if (i < infos[j].scores.size())
          context.resultCallback(infos[j].name, infos[j].scores[i]);
      context.leaveScope();
    }
  }
  
  /*
   ** Single Objective
   */
  void testSingleObjectiveOptimizers(ExecutionContext& context)
  {
    std::vector<ProblemPtr> problems;
    problems.push_back(new SphereProblem(numDims));
    problems.push_back(new AckleyProblem(numDims));
    problems.push_back(new GriewangkProblem(numDims));
    problems.push_back(new RastriginProblem(numDims));
    problems.push_back(new RosenbrockProblem(numDims));
    problems.push_back(new RosenbrockRotatedProblem(numDims));
    
    size_t numInitialSamples = 10 * numDims;
    
    std::vector<SolverSettings> solvers;
    
    // baseline solvers
    if (runBaseline)
    {
      solvers.push_back(createSettings(randomSolver(uniformScalarVectorSampler(), numEvaluationsBaseline), numEvaluationsBaseline, "Random search"));
      solvers.push_back(createSettings(randomSolver(diagonalGaussianSampler(), numEvaluationsBaseline), numEvaluationsBaseline, "Random gaussian search"));
      solvers.push_back(createSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluationsBaseline / populationSize), numEvaluationsBaseline, "Cross-entropy"));
      solvers.push_back(createSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluationsBaseline / populationSize, true), numEvaluationsBaseline, "Cross-entropy with elitism"));
      // TODO: add CMA-ES
    }
      
    
    // SBO solvers
    // create the splitting criterion    
    SplittingCriterionPtr splittingCriterion = stddevReductionSplittingCriterion();
    
    // create the sampler
    SamplerPtr testExpressionsSampler = subsetVectorSampler(scalarExpressionVectorSampler(), (size_t)(sqrt((double)numDims) + 0.5));

    // create inner optimization loop solver
    SolverPtr ceSolver = crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, 25, true);
    ceSolver->setVerbosity(verbosityQuiet);
    
    // Samplers
    SamplerPtr uniform = samplerToVectorSampler(uniformScalarVectorSampler(), numInitialSamples);
    SamplerPtr latinHypercube = latinHypercubeVectorSampler(numInitialSamples);
    
    // Variable Encoder
    VariableEncoderPtr encoder = scalarVectorVariableEncoder();
    
    // Selection Criteria
    SelectionCriterionPtr optimistic = optimisticSelectionCriterion(optimism);
    
    if (runRandomForests)
    {
      // create RF learner
      SolverPtr learner = treeLearner(splittingCriterion, exhaustiveConditionLearner(testExpressionsSampler)); 
      learner = baggingLearner(learner, numTrees);
      
      if (uniformSampling)
      {
        FitnessPtr bestPOI, bestEI;
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluationsSBO), numEvaluationsSBO, "SBO, RF, Optimistic, Uniform"));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluationsSBO), numEvaluationsSBO, "SBO, RF, Probability of Improvement, Uniform", &bestPOI));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluationsSBO), numEvaluationsSBO, "SBO, RF, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestPOI, bestEI;
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluationsSBO), numEvaluationsSBO, "SBO, RF, Optimistic, Latin Hypercube"));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluationsSBO), numEvaluationsSBO, "SBO, RF, Probability of Improvement, Latin Hypercube", &bestPOI));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluationsSBO), numEvaluationsSBO, "SBO, RF, Expected Improvement, Latin Hypercube", &bestEI));
      }
    } // runRandomForests
    
    if (runXT)
    {
      // create XT learner
      // these trees should choose random splits 
      SolverPtr learner = treeLearner(splittingCriterion, randomSplitConditionLearner(testExpressionsSampler)); 
      learner = simpleEnsembleLearner(learner, numTrees);
      
      if (uniformSampling)
      {
        FitnessPtr bestPOI, bestEI;
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluationsSBO), numEvaluationsSBO, "SBO, XT, Optimistic, Uniform"));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluationsSBO), numEvaluationsSBO, "SBO, XT, Probability of Improvement, Uniform", &bestPOI));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluationsSBO), numEvaluationsSBO, "SBO, XT, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestPOI, bestEI;
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluationsSBO), numEvaluationsSBO, "SBO, XT, Optimistic, Latin Hypercube"));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluationsSBO), numEvaluationsSBO, "SBO, XT, Probability of Improvement, Latin Hypercube", &bestPOI));
        solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluationsSBO), numEvaluationsSBO, "SBO, XT, Expected Improvement, Latin Hypercube", &bestEI));
      }
    } // runXT
    
    ProblemPtr problem = problems[problemIdx - 1];
    context.enterScope(problem->toShortString());
    context.resultCallback("problem", problem);
    std::vector<SolverInfo> infos;
    for (size_t j = 0; j < solvers.size(); ++j)
      infos.push_back(runSolver(context, problem, solvers[j]));
    context.enterScope("Results");
    displayResults(context, infos);
    context.leaveScope();
    context.leaveScope(); 
  }
};

}; /* namespace lbcpp */

#endif // !SBO_EXPERIMENTS_H_
