/*-----------------------------------------.---------------------------------.
| Filename: Solver.h                       | Base class for solvers          |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_H_
# define ML_SOLVER_H_

# include "predeclarations.h"
# include "SolverCallback.h"
# include "Problem.h"
# include "SolutionContainer.h"
# include "VariableEncoder.h"
# include "RandomVariable.h"
# include "Fitness.h"

namespace lbcpp
{

/*
** Verbosity
*/
enum SolverVerbosity
{
  verbosityQuiet = 0,
  verbosityProgressAndResult,
  verbosityDetailed,
  verbosityAll
};

/*
** Solver
*/
class Solver : public Object
{
public:
  Solver() : verbosity(verbosityQuiet) {}
  
  void setVerbosity(SolverVerbosity verbosity)
    {this->verbosity = verbosity;}

  SolverVerbosity getVerbosity() const
    {return verbosity;}

  void solve(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution = ObjectPtr());

  virtual void startBatch(ExecutionContext& context) {}
  virtual void stopBatch(ExecutionContext& context) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution = ObjectPtr());
  virtual void runSolver(ExecutionContext& context) = 0;
  virtual void stopSolver(ExecutionContext& context);
  
  ProblemPtr getProblem() const
    {return problem;}

  SolverCallbackPtr getCallback() const
    {return callback;}

  // utility functions to be called during solving
  FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
  void addSolution(ExecutionContext& context, const ObjectPtr& object, double fitness);  
  void addSolution(ExecutionContext& context, const ObjectPtr& object, const FitnessPtr& fitness);  

protected:
  typedef std::pair<ObjectPtr, FitnessPtr> SolutionAndFitnessPair;

  ProblemPtr problem;
  SolverCallbackPtr callback;
  SolverVerbosity verbosity;
};

extern SolverPtr nrpaSolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel);
extern SolverPtr beamNRPASolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel, size_t beamSizeAtFirstLevel, size_t beamSizeAtHigherLevels);

// learners
extern SolverPtr exhaustiveConditionLearner(SamplerPtr expressionsSampler);
extern SolverPtr randomSplitConditionLearner(SamplerPtr expressionsSampler);

extern SolverPtr simpleEnsembleLearner(const SolverPtr& baseLearner, size_t ensembleSize);
extern SolverPtr baggingLearner(const SolverPtr& baseLearner, size_t ensembleSize);

extern SolverPtr treeLearner(SplittingCriterionPtr splittingCriterion, SolverPtr conditionLearner, size_t minExamplesToSplit = 2, size_t maxDepth = 0);
extern SolverPtr sharkGaussianProcessLearner();

extern IterativeSolverPtr incrementalLearnerBasedLearner(IncrementalLearnerPtr learner);

class IterativeSolver : public Solver
{
public:
  IterativeSolver(size_t numIterations = 0)
    : numIterations(numIterations) {}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter) = 0; // returns false if the optimizer has converged

  virtual void runSolver(ExecutionContext& context);

protected:
  friend class IterativeSolverClass;

  size_t numIterations;
};

extern IterativeSolverPtr randomSolver(SamplerPtr sampler, size_t numIterations = 0);
extern IterativeSolverPtr repeatSolver(SolverPtr solver, size_t numIterations = 0);
extern IterativeSolverPtr lbfgsOptimizer(size_t numIterations = 0);
extern IterativeSolverPtr parEGOOptimizer(size_t numIterations = 0);

extern IterativeSolverPtr incrementalSurrogateBasedSolver(SamplerPtr initialVectorSampler, IncrementalLearnerPtr surrogateLearner, SolverPtr surrogateSolver, VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations = 0);
extern IterativeSolverPtr batchSurrogateBasedSolver(SamplerPtr initialVectorSampler, SolverPtr surrogateLearner, SolverPtr surrogateSolver, VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations = 0);

class PopulationBasedSolver : public IterativeSolver
{
public:
  PopulationBasedSolver(size_t populationSize = 100, size_t numGenerations = 0)
    : IterativeSolver(numGenerations), populationSize(populationSize) {}

protected:
  friend class PopulationBasedSolverClass;

  size_t populationSize;

  SolutionVectorPtr sampleAndEvaluatePopulation(ExecutionContext& context, SamplerPtr sampler, size_t populationSize);
  void computeMissingFitnesses(ExecutionContext& context, const SolutionVectorPtr& population);
  void learnSampler(ExecutionContext& context, SolutionVectorPtr solutions, SamplerPtr sampler);
};

extern PopulationBasedSolverPtr crossEntropySolver(SamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, SolutionComparatorPtr comparator = SolutionComparatorPtr());
extern PopulationBasedSolverPtr nsga2moOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9);
extern PopulationBasedSolverPtr cmaessoOptimizer(size_t populationSize = 100, size_t mu = 100, size_t numGenerations = 0);
extern PopulationBasedSolverPtr cmaesmoOptimizer(size_t populationSize = 100, size_t numOffsprings = 100, size_t numGenerations = 0);
  
}; /* namespace lbcpp */

#endif // !ML_OPTIMIZER_H_
