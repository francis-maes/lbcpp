/*-----------------------------------------.---------------------------------.
| Filename: Solver.h                       | Base class for solvers          |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_H_
# define LBCPP_ML_SOLVER_H_

# include "Problem.h"
# include "SolutionContainer.h"

namespace lbcpp
{

/*
** SolverCallback
*/
class SolverCallback : public Object
{
public:
  virtual void solverStarted(ExecutionContext& context, SolverPtr solver) {}
  virtual bool solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness) = 0; // return false to stop optimization
  virtual void solverStopped(ExecutionContext& context, SolverPtr solver) {}
};

extern SolverCallbackPtr storeBestFitnessSolverCallback(FitnessPtr& bestFitness);
extern SolverCallbackPtr fillParetoFrontSolverCallback(ParetoFrontPtr front);
extern SolverCallbackPtr maxEvaluationsSolverCallback(size_t maxEvaluations);

extern SolverCallbackPtr singleObjectiveEvaluatorSolverCallback(size_t evaluationPeriod, DenseDoubleVectorPtr cpuTimes, DenseDoubleVectorPtr scores);
extern SolverCallbackPtr hyperVolumeEvaluatorSolverCallback(size_t evaluationPeriod, DenseDoubleVectorPtr cpuTimes, DenseDoubleVectorPtr scores);

extern SolverCallbackPtr compositeSolverCallback(SolverCallbackPtr callback1, SolverCallbackPtr callback2);
extern SolverCallbackPtr compositeSolverCallback(SolverCallbackPtr callback1, SolverCallbackPtr callback2, SolverCallbackPtr callback3);
  
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

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution = ObjectPtr());
  virtual void runSolver(ExecutionContext& context) = 0;
  virtual void stopSolver(ExecutionContext& context);
  
  ProblemPtr getProblem() const
    {return problem;}

  SolverCallbackPtr getCallback() const
    {return callback;}
  
protected:
  typedef std::pair<ObjectPtr, FitnessPtr> SolutionAndFitnessPair;

  ProblemPtr problem;
  SolverCallbackPtr callback;
  SolverVerbosity verbosity;
  bool shouldStop;

  FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
};

extern SolverPtr nrpaSolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel);
extern SolverPtr beamNRPASolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel, size_t beamSizeAtFirstLevel, size_t beamSizeAtHigherLevels);

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

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_H_
