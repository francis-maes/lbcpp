/*-----------------------------------------.---------------------------------.
 | Filename: SBOEnsembleSizeExperiments     | Surrogate-Based Optimization    |
 | Author  : Denny Verbeeck                 | Ensemble Size Experimental      |
 | Started : 04/03/2013 17:20               | Evaluation                      |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef SBO_ENSEMBLE_SIZE_EXPERIMENTS_H_
# define SBO_ENSEMBLE_SIZE_EXPERIMENTS_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/RandomVariable.h>
# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>
# include <ml/ExpressionSampler.h>

# include <ml/SplittingCriterion.h>
# include <ml/SelectionCriterion.h>

# include "SharkProblems.h"
# include "SolverInfo.h"

namespace lbcpp
{
  
extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class SBOEnsembleSizeExperiments : public WorkUnit
{
public:
  SBOEnsembleSizeExperiments() :  numEvaluations(1000),
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

    context.getRandomGenerator()->setSeed(1564);

    testSingleObjectiveOptimizers(context);
    //testBiObjectiveOptimizers(context);
    return ObjectPtr();
  }
  
protected:
  friend class SBOEnsembleSizeExperimentsClass;
  
  size_t numEvaluations;         /**< Number of evaluations                                            */

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
    size_t populationSize = 10 * numDims;
    std::vector<SolverSettings> solvers;

    // SBO solvers
    // create the splitting criterion    
    SplittingCriterionPtr splittingCriterion = stddevReductionSplittingCriterion();
    
    // create the sampler
    SamplerPtr testExpressionsSampler = subsetVectorSampler(scalarExpressionVectorSampler(), (size_t)(sqrt((double)numDims) + 0.5));

    // create inner optimization loop solver
    //SolverPtr innerSolver = cmaessoOptimizer(100);
    SolverPtr innerSolver = crossEntropySolver(diagonalGaussianSampler(), numDims * 10, numDims * 3, 20);
    innerSolver->setVerbosity(verbosityQuiet);
    
    // Variable Encoder
    VariableEncoderPtr encoder = scalarVectorVariableEncoder();
    
    // Samplers
    SamplerPtr latinHypercubeModified = latinHypercubeVectorSampler(numInitialSamples, true);

    // baseline solvers
    solvers.push_back(SolverSettings(randomSolver(uniformSampler(), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Random search"));
    solvers.push_back(SolverSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Cross-entropy"));
    solvers.push_back(SolverSettings(crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, numEvaluations / populationSize, true), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "Cross-entropy with elitism"));
    solvers.push_back(SolverSettings(cmaessoOptimizer(numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "CMA-ES"));

    size_t factors[5] = {1, 5, 10, 20, 50};
    for (size_t i = 0; i < 5; ++i)
    {
      FitnessPtr bestEI;
      size_t numTrees = numDims * factors[i];
      IncrementalLearnerPtr xtIncrementalLearner = new EnsembleIncrementalLearner(new PureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      solvers.push_back(SolverSettings(incrementalSurrogateBasedSolver(latinHypercubeModified, xtIncrementalLearner, innerSolver, encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "IXT(" + string((int)numTrees) + ")", &bestEI)); 
    }
    
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
    default: jassertfalse; return ProblemPtr();
    };
  }
};

}; /* namespace lbcpp */

#endif // !SBO_ENSEMBLE_SIZE_EXPERIMENTS_H_
