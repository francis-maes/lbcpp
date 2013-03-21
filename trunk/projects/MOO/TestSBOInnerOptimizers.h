/*-----------------------------------------.---------------------------------.
 | Filename: TestSBOInnerOptimizers.h      | WorkUnit for comparing inner    |
 | Author  : Denny Verbeeck                | optimizers in surrogate-based   |
 | Started : 14/03/2013 16:05              | optimization                    |
 `-----------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef TEST_SBO_INNER_OPTIMIZERS_H_
# define TEST_SBO_INNER_OPTIMIZERS_H_

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

class TestSBOInnerOptimizers : public WorkUnit
{
public:
  TestSBOInnerOptimizers() : numEvaluations(200), numTrees(100), numDims(6), numRuns(10) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    testSingleObjectiveOptimizers(context);
    //testBiObjectiveOptimizers(context);
    return ObjectPtr();
  }
  
protected:
  friend class TestSBOInnerOptimizersClass;
  
  // options for baseline algorithms
  size_t numEvaluations;         /**< Number of evaluations                                            */
  
  size_t numTrees;               /**< Size of forest for Random Forest and extremely randomized trees  */
  
  size_t numDims;                /**< Number of dimensions of the decision space                       */
  size_t numRuns;                /**< Number of runs to average over                                   */
  
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
    size_t populationSize = 10 * numDims;
    double evaluationPeriod = 5.0;
    double evaluationPeriodFactor = 1.0;
    size_t verbosity = 1;
    size_t optimizerVerbosity = 1;
    
    std::vector<SolverSettings> solvers;
        
    // SBO solvers
    // create the splitting criterion    
    SplittingCriterionPtr splittingCriterion = stddevReductionSplittingCriterion();
    
    // create the sampler
    SamplerPtr testExpressionsSampler = subsetVectorSampler(scalarExpressionVectorSampler(), (size_t)(sqrt((double)numDims) + 0.5));
    
    // create inner optimization loop solvers
    std::vector<SolverPtr> innerSolvers;
    SolverPtr solver = crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, 25, true);
    solver->setVerbosity(verbosityQuiet);
    innerSolvers.push_back(solver);
    
    solver = cmaessoOptimizer(25);
    solver->setVerbosity(verbosityQuiet);
    innerSolvers.push_back(solver);
    
    // Samplers
    SamplerPtr uniform = samplerToVectorSampler(uniformScalarVectorSampler(), numInitialSamples);
    SamplerPtr latinHypercube = latinHypercubeVectorSampler(numInitialSamples);
    
    // Variable Encoder
    VariableEncoderPtr encoder = scalarVectorVariableEncoder();
    
    SolverPtr learner = treeLearner(splittingCriterion, exhaustiveConditionLearner(testExpressionsSampler)); 
    learner = baggingLearner(learner, numTrees);
    
    {
      // create RF learner
      SolverPtr learner = treeLearner(splittingCriterion, exhaustiveConditionLearner(testExpressionsSampler)); 
      learner = baggingLearner(learner, numTrees);
      for (size_t i = 0; i < innerSolvers.size(); ++i)
      {
        FitnessPtr bestEI;
        solvers.push_back(SolverSettings(batchSurrogateBasedSolver(latinHypercube, learner, innerSolvers[i], encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, RF, Expected Improvement, Latin Hypercube, " + innerSolvers[i]->toShortString(), &bestEI));
      }
    }
    
    {
      // create XT learner
      // these trees should choose random splits 
      SolverPtr learner = treeLearner(splittingCriterion, randomSplitConditionLearner(testExpressionsSampler)); 
      learner = simpleEnsembleLearner(learner, numTrees);
      for (size_t i = 0; i < innerSolvers.size(); ++i)
      {
        FitnessPtr bestEI;
        solvers.push_back(SolverSettings(batchSurrogateBasedSolver(latinHypercube, learner, innerSolvers[i], encoder, expectedImprovementSelectionCriterion(bestEI), numEvaluations), numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "SBO, XT, Expected Improvement, Latin Hypercube, " + innerSolvers[i]->toShortString(), &bestEI));
      }
    }
    
    for (size_t i = 0; i < problems.size(); ++i)
    {
      ProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());
      context.resultCallback("problem", problem);
      std::vector<SolverInfo> infos;
      for (size_t j = 0; j < solvers.size(); ++j)
        infos.push_back(solvers[j].runSolver(context, problem));
      SolverInfo::displayResults(context, infos);
      context.leaveScope();
    }
  }
};

}; /* namespace lbcpp */

#endif // !TEST_SBO_INNER_OPTIMIZERS_H_
