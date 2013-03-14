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
  SBOExperiments() :  numEvaluations(1000),
                      runBaseline(false),
                      populationSize(20),
                      runRandomForests(false),
                      runXT(false),
                      uniformSampling(false),
                      latinHypercubeSampling(false),
                      numTrees(100),
                      optimism(2.0),
                      evaluationPeriod(10.0),
                      evaluationPeriodFactor(1.0),
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
  size_t numEvaluations;         /**< Number of evaluations                                            */
  bool runBaseline;              /**< Run baseline algorithms                                          */
  size_t populationSize;         /**< Population size for baseline optimizers                          */
  
  // options for surrogate-based algorithms
  bool runRandomForests;         /**< Run with random forest surrogate                                 */
  bool runXT;                    /**< Run with extremely randomized trees                              */

  bool uniformSampling;          /**< Run with uniform sampling                                        */
  bool latinHypercubeSampling;   /**< Run with latin hypercube sampling                                */

  size_t numTrees;               /**< Size of forest for Random Forest and extremely randomized trees  */
  
  double optimism;               /**< Level of optimism for optimistic surrogate-based optimizers      */

  // general options
  double evaluationPeriod;       /**< Number of seconds between evaluations in function of CPU time    */
  double evaluationPeriodFactor; /**< After each evaluation for CPU time, evaluationPeriod is
                                      multiplied by this factor                                        */
  
  size_t numDims;                /**< Number of dimensions of the decision space                       */
  size_t numRuns;                /**< Number of runs to average over                                   */
  
  size_t verbosity;
  size_t optimizerVerbosity;
  
  size_t problemIdx;             /**< The problem to run (1-6)                                         */
  
  struct ResultCurve
  {
    IVectorPtr evaluations;
    DVectorPtr cpuTimes;
    DVectorPtr scores;
    
    void initialize()
    {
      evaluations = new IVector();
      cpuTimes = new DVector();
      scores = new DVector();
    }
  };
  
  struct SolverInfo
  {
    string name;
    ResultCurve inFunctionOfEvaluations;
    ResultCurve inFunctionOfCpuTime;
    
    void initialize(string name)
    {
      this->name = name;
      inFunctionOfEvaluations.initialize();
      inFunctionOfCpuTime.initialize();
    }
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
    res.initialize(solverSettings.description);
    double best = mergeResults(res, runInfos);
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
    
    IVectorPtr evaluations = new IVector();
    DVectorPtr cpuTimes = new DVector();
    DVectorPtr scores = new DVector();
    SolverEvaluatorPtr evaluator = singleObjectiveSolverEvaluator(*solverSettings.bestFitness);
    
    SolverCallbackPtr callback1 = compositeSolverCallback(storeBestFitnessSolverCallback(*solverSettings.bestFitness), 
                                                          evaluationPeriodEvaluatorSolverCallback(evaluator, evaluations, cpuTimes, scores, 2));
    
    info.inFunctionOfEvaluations.evaluations = evaluations;
    info.inFunctionOfEvaluations.cpuTimes = cpuTimes;
    info.inFunctionOfEvaluations.scores = scores;

    evaluations = new IVector();
    cpuTimes = new DVector();
    scores = new DVector();
    SolverCallbackPtr callback2 = compositeSolverCallback(logTimePeriodEvaluatorSolverCallback(evaluator, evaluations, cpuTimes, scores, evaluationPeriod, evaluationPeriodFactor), maxEvaluationsSolverCallback(solverSettings.numEvaluations));

    info.inFunctionOfCpuTime.evaluations = evaluations;
    info.inFunctionOfCpuTime.cpuTimes = cpuTimes;
    info.inFunctionOfCpuTime.scores = scores;
    
    solverSettings.solver->setVerbosity((SolverVerbosity)optimizerVerbosity);
    solverSettings.solver->solve(context, problem, compositeSolverCallback(callback1, callback2));
    
    context.resultCallback("optimizer", solverSettings.solver);
    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("curveWithFixedEvaluations");
      for (size_t i = 0; i < info.inFunctionOfEvaluations.evaluations->getNumElements(); ++i)
      {
        context.enterScope(string(info.inFunctionOfEvaluations.evaluations->get(i)));
        context.resultCallback("numEvaluations", info.inFunctionOfEvaluations.evaluations->get(i));
        context.resultCallback("score", info.inFunctionOfEvaluations.scores->get(i));
        if (info.inFunctionOfEvaluations.cpuTimes->get(i) != DVector::missingValue)
          context.resultCallback("cpuTime", info.inFunctionOfEvaluations.cpuTimes->get(i));
        context.leaveScope();
      }
      context.leaveScope();
      context.enterScope("curveWithFixedCpuTime");
      for (size_t i = 0; i < info.inFunctionOfCpuTime.cpuTimes->getNumElements(); ++i)
      {
        context.enterScope(string(info.inFunctionOfCpuTime.cpuTimes->get(i)));
        context.resultCallback("cpuTime", info.inFunctionOfCpuTime.cpuTimes->get(i));
        if (info.inFunctionOfCpuTime.scores->get(i) != DVector::missingValue)
          context.resultCallback("score", info.inFunctionOfCpuTime.scores->get(i));
        context.resultCallback("numEvaluations", info.inFunctionOfCpuTime.evaluations->get(i));
        context.leaveScope();
      }
      context.leaveScope();
    }
    return (*solverSettings.bestFitness)->toDouble();
  }
  
  double mergeResults(SolverInfo& res, const std::vector<SolverInfo>& infos)
  {
    double best = DBL_MAX;
    size_t maxLengthEvaluations = 0;
    size_t maxLengthCpuTimes = 0;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      maxLengthEvaluations = (infos[i].inFunctionOfEvaluations.evaluations->getNumElements() > maxLengthEvaluations ? infos[i].inFunctionOfEvaluations.evaluations->getNumElements() : maxLengthEvaluations);
      maxLengthCpuTimes = (infos[i].inFunctionOfCpuTime.cpuTimes->getNumElements() > maxLengthCpuTimes ? infos[i].inFunctionOfCpuTime.cpuTimes->getNumElements() : maxLengthCpuTimes);
    }
    for (size_t i = 0; i < maxLengthEvaluations; ++i)
    {
      ScalarVariableMean meanScore;
      ScalarVariableMean meanCpuTime;
      size_t evaluations = 0;
      for (size_t j = 0; j < infos.size(); ++j)
      {
        ResultCurve curve = infos[j].inFunctionOfEvaluations;
        if (i < curve.scores->getNumElements())
        {
          meanScore.push(curve.scores->get(i));
          meanCpuTime.push(curve.cpuTimes->get(i));
          if (evaluations)
            jassert(evaluations == curve.evaluations->get(i)); // if this fails there is smth wrong in EvaluationPeriodEvaluatorSolverCallback, results not aligned on nb of evaluations
          evaluations = curve.evaluations->get(i);
        }
        else
        {
          meanScore.push(curve.scores->get(curve.scores->getNumElements() - 1));
          meanCpuTime.push(curve.cpuTimes->get(curve.cpuTimes->getNumElements() - 1));
        }
      }
      res.inFunctionOfEvaluations.evaluations->append(evaluations);
      res.inFunctionOfEvaluations.scores->append(meanScore.getCount() ? meanScore.getMean() : DVector::missingValue);
      res.inFunctionOfEvaluations.cpuTimes->append(meanCpuTime.getCount() ? meanCpuTime.getMean() : DVector::missingValue);
      best = (best < res.inFunctionOfEvaluations.scores->get(i) ? best : res.inFunctionOfEvaluations.scores->get(i));
    }
    for (size_t i = 0; i < maxLengthCpuTimes; ++i)
    {
      ScalarVariableMean meanScore;
      ScalarVariableMean meanEvaluations;
      double cpuTime = 0.0;
      for (size_t j = 0; j < infos.size(); ++j)
      {
        ResultCurve curve = infos[j].inFunctionOfCpuTime;
        if (i < curve.scores->getNumElements())
        {
          if (curve.scores->get(i) != DVector::missingValue)
            meanScore.push(curve.scores->get(i));
          if (curve.evaluations->get(i) != DVector::missingValue)
            meanEvaluations.push(curve.evaluations->get(i));
          if (cpuTime != 0.0)
            jassert(fabs(cpuTime - curve.cpuTimes->get(i)) < 1e-5); // if this fails there is smth wrong in TimePeriodEvaluatorSolverCallback, results not aligned on cputime
          cpuTime = curve.cpuTimes->get(i);
        }
        else
        {
          meanScore.push(curve.scores->get(curve.scores->getNumElements() - 1));
          meanEvaluations.push(curve.evaluations->get(curve.evaluations->getNumElements() - 1));
        }
      }
      res.inFunctionOfCpuTime.evaluations->append(meanEvaluations.getCount() ? meanEvaluations.getMean() : DVector::missingValue);
      res.inFunctionOfCpuTime.scores->append(meanScore.getCount() ? meanScore.getMean() : DVector::missingValue);
      res.inFunctionOfCpuTime.cpuTimes->append(cpuTime);
      best = (best < res.inFunctionOfCpuTime.scores->get(i) ? best : res.inFunctionOfCpuTime.scores->get(i));
    }
    return best;
  }
  
  void displayResults(ExecutionContext& context, const std::vector<SolverInfo>& infos)
  {
    size_t maxLengthEvaluations = 0;
    size_t maxLengthCpuTimes = 0;
    IVectorPtr longestEvaluationsVector;
    DVectorPtr longestCpuTimesVector;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      if (infos[i].inFunctionOfEvaluations.scores->getNumElements() > maxLengthEvaluations)
      {
        maxLengthEvaluations = infos[i].inFunctionOfEvaluations.evaluations->getNumElements();
        longestEvaluationsVector = infos[i].inFunctionOfEvaluations.evaluations;
      }
      if (infos[i].inFunctionOfCpuTime.scores->getNumElements() > maxLengthCpuTimes)
      {
        maxLengthCpuTimes = infos[i].inFunctionOfCpuTime.cpuTimes->getNumElements();
        longestCpuTimesVector = infos[i].inFunctionOfCpuTime.cpuTimes;
      }
    }
    context.enterScope("Results(evaluations)");
    for (size_t i = 0; i < maxLengthEvaluations; ++i)
    {
      context.enterScope(string(longestEvaluationsVector->get(i)));
      context.resultCallback("numEvaluations", longestEvaluationsVector->get(i));
      if (longestEvaluationsVector->get(i))
        context.resultCallback("log(numEvaluations)", log10((double)longestEvaluationsVector->get(i)));
      for (size_t j = 0; j < infos.size(); ++j)
      {
        if (i < infos[j].inFunctionOfEvaluations.scores->getNumElements() &&
            infos[j].inFunctionOfEvaluations.scores->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name + ".score", infos[j].inFunctionOfEvaluations.scores->get(i));
        if (i < infos[j].inFunctionOfEvaluations.cpuTimes->getNumElements() &&
            infos[j].inFunctionOfEvaluations.cpuTimes->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name + ".cpuTime", infos[j].inFunctionOfEvaluations.cpuTimes->get(i));
      }
      context.leaveScope();
    }
    context.leaveScope();
    context.enterScope("Results(CpuTime)");
    for (size_t i = 0; i < maxLengthCpuTimes; ++i)
    {
      context.enterScope(string(longestCpuTimesVector->get(i)));
      context.resultCallback("cpuTime", longestCpuTimesVector->get(i));
      if (longestCpuTimesVector->get(i))
        context.resultCallback("log(cpuTime)", log10((double)longestCpuTimesVector->get(i)));
      for (size_t j = 0; j < infos.size(); ++j)
      {
        if (i < infos[j].inFunctionOfCpuTime.scores->getNumElements() &&
            infos[j].inFunctionOfCpuTime.scores->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name + ".score", infos[j].inFunctionOfCpuTime.scores->get(i));
        if (i < infos[j].inFunctionOfCpuTime.evaluations->getNumElements() &&
            infos[j].inFunctionOfCpuTime.evaluations->get(i) != DVector::missingValue)
          context.resultCallback(infos[j].name + ".evaluations", infos[j].inFunctionOfCpuTime.evaluations->get(i));
      }
      context.leaveScope();
    }
    context.leaveScope();
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
      solvers.push_back(createSettings(randomSolver(uniformScalarVectorSampler(), numEvaluations), numEvaluations, "Random search"));
      solvers.push_back(createSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize), numEvaluations, "Cross-entropy"));
      solvers.push_back(createSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize, true), numEvaluations, "Cross-entropy with elitism"));
      solvers.push_back(createSettings(cmaessoOptimizer(numEvaluations), numEvaluations, "CMA-ES"));
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
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, RF, Optimistic, Uniform"));
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, RF, Probability of Improvement, Uniform", &bestPOI));
        solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numEvaluations, "SBO, RF, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestPOI, bestEI;
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, RF, Optimistic, Latin Hypercube"));
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, RF, Probability of Improvement, Latin Hypercube", &bestPOI));
        solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numEvaluations, "SBO, RF, Expected Improvement, Latin Hypercube", &bestEI));
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
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, XT, Optimistic, Uniform"));
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, XT, Probability of Improvement, Uniform", &bestPOI));
        solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numEvaluations, "SBO, XT, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestPOI, bestEI;
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, XT, Optimistic, Latin Hypercube"));
        //solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, XT, Probability of Improvement, Latin Hypercube", &bestPOI));
        solvers.push_back(createSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numEvaluations, "SBO, XT, Expected Improvement, Latin Hypercube", &bestEI));
      }
    } // runXT
    
    ProblemPtr problem = problems[problemIdx - 1];
    context.enterScope(problem->toShortString());
    context.resultCallback("problem", problem);
    std::vector<SolverInfo> infos;
    for (size_t j = 0; j < solvers.size(); ++j)
      infos.push_back(runSolver(context, problem, solvers[j]));
    displayResults(context, infos);
    context.leaveScope(); 
  }
};

}; /* namespace lbcpp */

#endif // !SBO_EXPERIMENTS_H_
