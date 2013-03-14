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
# include "SolverInfo.h"

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
      solvers.push_back(SolverSettings(randomSolver(uniformScalarVectorSampler(), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Random search"));
      solvers.push_back(SolverSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Cross-entropy"));
      solvers.push_back(SolverSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize, true), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Cross-entropy with elitism"));
      solvers.push_back(SolverSettings(cmaessoOptimizer(numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "CMA-ES"));
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
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, RF, Optimistic, Uniform"));
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, RF, Probability of Improvement, Uniform", &bestPOI));
        solvers.push_back(SolverSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, RF, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestPOI, bestEI;
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, RF, Optimistic, Latin Hypercube"));
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, RF, Probability of Improvement, Latin Hypercube", &bestPOI));
        solvers.push_back(SolverSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, RF, Expected Improvement, Latin Hypercube", &bestEI));
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
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, XT, Optimistic, Uniform"));
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, XT, Probability of Improvement, Uniform", &bestPOI));
        solvers.push_back(SolverSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, XT, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestPOI, bestEI;
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, optimistic, numEvaluations), numEvaluations, "SBO, XT, Optimistic, Latin Hypercube"));
        //solvers.push_back(createSettings(surrogateBasedSolver(uniform, learner, ceSolver, encoder, probabilityOfImprovementSelectionCriterion(bestPOI), numEvaluations), numEvaluations, "SBO, XT, Probability of Improvement, Latin Hypercube", &bestPOI));
        solvers.push_back(SolverSettings(batchSurrogateBasedSolver(uniform, learner, ceSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, XT, Expected Improvement, Latin Hypercube", &bestEI));
      }
    } // runXT
    
    ProblemPtr problem = problems[problemIdx - 1];
    context.enterScope(problem->toShortString());
    context.resultCallback("problem", problem);
    std::vector<SolverInfo> infos;
    for (size_t j = 0; j < solvers.size(); ++j)
      infos.push_back(solvers[j].runSolver(context, problem));
    SolverInfo::displayResults(context, infos);
    context.leaveScope(); 
  }
};

}; /* namespace lbcpp */

#endif // !SBO_EXPERIMENTS_H_
