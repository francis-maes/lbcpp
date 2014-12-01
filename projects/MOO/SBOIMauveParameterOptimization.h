/*-----------------------------------------.---------------------------------.
 | Filename: SBOImauveParameterOptimization | Finding optimal parameters for  |
 | Author  : Denny Verbeeck                 | iMauve in SBO                   |
 | Started : 04/03/2013 17:20               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef SBO_IMAUVE_PARAMATER_OPTIMIZATION_H_
# define SBO_IMAUVE_PARAMATER_OPTIMIZATION_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/RandomVariable.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>

# include <ml/SplittingCriterion.h>
# include <ml/SelectionCriterion.h>

# include "SharkProblems.h"
# include "SolverInfo.h"

namespace lbcpp
{
  
extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class SBOIMauveParameterOptimization : public WorkUnit
{
public:
  SBOIMauveParameterOptimization() :  numEvaluations(1000),
                      runBaseline(false),
                      populationSize(20),
                      uniformSampling(false),
                      latinHypercubeSampling(false),
                      modifiedLatinHypercubeSampling(false),
                      edgeSampling(false),
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
    jassert(problemIdx >= 1 && problemIdx <= 7);

    context.getRandomGenerator()->setSeed(1664);

    testSingleObjectiveOptimizers(context);
    //testBiObjectiveOptimizers(context);
    return ObjectPtr();
  }
  
protected:
  friend class SBOIMauveParameterOptimizationClass;
  
  // options for baseline algorithms
  size_t numEvaluations;         /**< Number of evaluations                                            */
  bool runBaseline;              /**< Run baseline algorithms                                          */
  size_t populationSize;         /**< Population size for baseline optimizers                          */
  
  bool uniformSampling;          /**< Run with uniform sampling                                        */
  bool latinHypercubeSampling;   /**< Run with latin hypercube sampling                                */
  bool modifiedLatinHypercubeSampling; /**< Run with modified latin hypercube sampling                 */
  bool edgeSampling;             /**< Run with edge sampling                                           */

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
    size_t numInitialSamples = 10 * numDims;
    
    std::vector<SolverSettings> solvers;

    // SBO solvers
    // create the splitting criterion    
    SplittingCriterionPtr splittingCriterion = stddevReductionSplittingCriterion();

    // create inner optimization loop solver
    //SolverPtr innerSolver = cmaessoOptimizer(100);
    SolverPtr innerSolver = crossEntropySolver(diagonalGaussianSampler(), numDims * 10, numDims * 3, 20);
    innerSolver->setVerbosity((SolverVerbosity)optimizerVerbosity);
    
    // Variable Encoder
    VariableEncoderPtr encoder = scalarVectorVariableEncoder();
    
    // Samplers
    SamplerPtr uniform = samplerToVectorSampler(uniformSampler(), numInitialSamples);
    SamplerPtr latinHypercube = latinHypercubeVectorSampler(numInitialSamples);
    SamplerPtr latinHypercubeModified = latinHypercubeVectorSampler(numInitialSamples, true);
    SamplerPtr edgeSampler = edgeVectorSampler();

    // baseline solvers
    if (runBaseline)
    {
      solvers.push_back(SolverSettings(randomSolver(uniformSampler(), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Random search"));
      solvers.push_back(SolverSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Cross-entropy"));
      solvers.push_back(SolverSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize, true), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Cross-entropy with elitism"));
      solvers.push_back(SolverSettings(cmaessoOptimizer(populationSize, populationSize / 2, numEvaluations / populationSize), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "CMA-ES"));
    }

    size_t chunkSize;
    double delta, threshold;
    std::vector<size_t> chunkSizes;
    chunkSizes.push_back(2 * numDims);
    chunkSizes.push_back(5 * numDims);
    chunkSizes.push_back(10 * numDims);
    std::vector<double> deltas;
    deltas.push_back(0.005);
    deltas.push_back(0.01);
    deltas.push_back(0.05);
    deltas.push_back(0.10);
    deltas.push_back(0.20);
    std::vector<double> thresholds;
    thresholds.push_back(0.01);
    thresholds.push_back(0.05);
    thresholds.push_back(0.10);
    thresholds.push_back(0.20);

    for (size_t i = 0; i < chunkSizes.size(); ++i)
    for (size_t j = 0; j < deltas.size(); ++j)
    for (size_t k = 0; k < thresholds.size(); ++k)
    {
      chunkSize = chunkSizes[i];
      delta = deltas[j];
      threshold = thresholds[k];
      // create iMauve learner
      IncrementalLearnerPtr learner = hoeffdingTreeIncrementalLearner(hoeffdingBoundTotalMauveIncrementalSplittingCriterion(chunkSize, delta, threshold), linearLeastSquaresRegressionIncrementalLearner());
      
      if (uniformSampling)
      {
        FitnessPtr bestEI;
        solvers.push_back(SolverSettings(incrementalSurrogateBasedSolver(uniform, learner, innerSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, iMauve, Expected Improvement, Uniform", &bestEI));
      }
      
      if (latinHypercubeSampling)
      {
        FitnessPtr bestEI;
        solvers.push_back(SolverSettings(incrementalSurrogateBasedSolver(latinHypercube, learner, innerSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, iMauve, Expected Improvement, Latin Hypercube", &bestEI));
      }
      
      if (modifiedLatinHypercubeSampling)
      {
        FitnessPtr bestEI;
        solvers.push_back(SolverSettings(incrementalSurrogateBasedSolver(latinHypercubeModified, learner, innerSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, iMauve, Expected Improvement, Modified Latin Hypercube", &bestEI));
      }
      
      if (edgeSampling)
      {
        FitnessPtr bestEI;
        solvers.push_back(SolverSettings(incrementalSurrogateBasedSolver(edgeSampler, learner, innerSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, iMauve, Expected Improvement, Edge Sampling", &bestEI));        
      }
    } // runIMauve
    
    std::vector<ProblemPtr> problemVariants(numRuns);
    for (size_t i = 0; i < problemVariants.size(); ++i)
      problemVariants[i] = createProblem(problemIdx - 1);

    context.enterScope(problemVariants[0]->toShortString());
    std::vector<SolverInfo> infos;
    for (size_t j = 0; j < solvers.size(); ++j)
      infos.push_back(solvers[j].runSolver(context, problemVariants));
    SolverInfo::displayResults(context, infos);
    context.leaveScope(); 
  }

  ProblemPtr createProblem(size_t index) const
  {
    switch (index)
    {
    case 0: return new SphereProblem(numDims);
    case 1: return new AckleyProblem(numDims);
    case 2: return new GriewangkProblem(numDims);
    case 3: return new RastriginProblem(numDims);
    case 4: return new RosenbrockProblem(numDims);
    case 5: return new RosenbrockRotatedProblem(numDims);
    case 6: return new ZDT1MOProblem(numDims);
    default: jassertfalse; return ProblemPtr();
    };
  }
};

}; /* namespace lbcpp */

#endif // !SBO_IMAUVE_PARAMATER_OPTIMIZATION_H_
