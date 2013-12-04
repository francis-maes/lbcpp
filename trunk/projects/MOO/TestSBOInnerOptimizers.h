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

class TestOneOptimizer : public WorkUnit
{
public:
  TestOneOptimizer(SolverSettings* settings = NULL, ProblemPtr problem = ProblemPtr()) : settings(settings), problem(problem) {}
  virtual ObjectPtr run(ExecutionContext& context)
  {
    std::vector<ProblemPtr> problems(1);
    problems[0] = problem;
    SolverInfoPtr res = new SolverInfo(settings->runSolver(context, problem));
    return res;
  }

  virtual string toShortString() const
    {return problem->toShortString() + ", " + settings->getDescription();}

private:
  SolverSettings* settings;
  ProblemPtr problem;
};

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
    double evaluationPeriod = 10.0;
    double evaluationPeriodFactor = 1.0;
    size_t verbosity = 1;
    size_t optimizerVerbosity = 1;
    
    CompositeWorkUnitPtr subWorkUnits(new CompositeWorkUnit(T("Test Inner Optimizers")));

    const size_t numSolvers = 4;
    std::vector<FitnessPtr> fitnessptrs(numSolvers * problems.size());
    for (size_t i = 0; i < numSolvers * problems.size(); ++i)
      fitnessptrs[i] = FitnessPtr();

    for (size_t i = 0; i < problems.size(); ++i)
    {
      // cross-entropy, 25 generations
      FitnessPtr& bestEI1 = fitnessptrs[numSolvers * i];
      SolverPtr solver = crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, 25, true);
      solver->setVerbosity(verbosityQuiet);
      IncrementalLearnerPtr xtIncrementalLearner = ensembleIncrementalLearner(pureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      subWorkUnits->addWorkUnit(new TestOneOptimizer(new SolverSettings(incrementalSurrogateBasedSolver(latinHypercubeVectorSampler(numInitialSamples, true), 
                                                                        xtIncrementalLearner, 
                                                                        solver, 
                                                                        scalarVectorVariableEncoder(), 
                                                                        expectedImprovementSelectionCriterion(bestEI1), 
                                                                        numEvaluations), 
                                        numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "IXT(" + string((int)numTrees) + "), CE(25)", &bestEI1),
                                problems[i]));

      // cross-entropy, 100 generations
      FitnessPtr& bestEI4 = fitnessptrs[numSolvers * i + 3];
      solver = crossEntropySolver(diagonalGaussianSampler(), populationSize, populationSize / 3, 100, true);
      solver->setVerbosity(verbosityQuiet);
      xtIncrementalLearner = ensembleIncrementalLearner(pureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      subWorkUnits->addWorkUnit(new TestOneOptimizer(new SolverSettings(incrementalSurrogateBasedSolver(latinHypercubeVectorSampler(numInitialSamples, true), 
                                                                        xtIncrementalLearner, 
                                                                        solver, 
                                                                        scalarVectorVariableEncoder(), 
                                                                        expectedImprovementSelectionCriterion(bestEI4), 
                                                                        numEvaluations), 
                                        numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "IXT(" + string((int)numTrees) + "), CE(100)", &bestEI4),
                                problems[i]));

      // cma-es, 100 evaluations
      FitnessPtr& bestEI2 = fitnessptrs[numSolvers * i + 1];
      solver = cmaessoOptimizer(100);
      solver->setVerbosity(verbosityQuiet);
      xtIncrementalLearner = ensembleIncrementalLearner(pureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      subWorkUnits->addWorkUnit(new TestOneOptimizer(new SolverSettings(incrementalSurrogateBasedSolver(latinHypercubeVectorSampler(numInitialSamples, true), 
                                                                        xtIncrementalLearner, 
                                                                        solver, 
                                                                        scalarVectorVariableEncoder(), 
                                                                        expectedImprovementSelectionCriterion(bestEI2), 
                                                                        numEvaluations), 
                                        numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "IXT(" + string((int)numTrees) + "), CMA-ES(100)", &bestEI2),
                                problems[i]));

      // cma-es, 500 evaluations
      FitnessPtr& bestEI3 = fitnessptrs[numSolvers * i + 2];
      solver = cmaessoOptimizer(500);
      solver->setVerbosity(verbosityQuiet);
      xtIncrementalLearner = ensembleIncrementalLearner(pureRandomScalarVectorTreeIncrementalLearner(), numTrees);
      subWorkUnits->addWorkUnit(new TestOneOptimizer(new SolverSettings(incrementalSurrogateBasedSolver(latinHypercubeVectorSampler(numInitialSamples, true), 
                                                                        xtIncrementalLearner, 
                                                                        solver, 
                                                                        scalarVectorVariableEncoder(), 
                                                                        expectedImprovementSelectionCriterion(bestEI3), 
                                                                        numEvaluations), 
                                        numRuns, numEvaluations, evaluationPeriod, evaluationPeriodFactor, verbosity, optimizerVerbosity, "IXT(" + string((int)numTrees) + "), CMA-ES(500)", &bestEI3),
                                problems[i]));
    }
    subWorkUnits->setPushChildrenIntoStackFlag(true);
    OVectorPtr res = context.run(subWorkUnits).staticCast<OVector>();
  }
};

}; /* namespace lbcpp */

#endif // !TEST_SBO_INNER_OPTIMIZERS_H_
