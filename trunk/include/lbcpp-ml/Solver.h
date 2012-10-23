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

namespace lbcpp
{

class Solver : public Object
{
public:
  Solver() : verbosity(verbosityQuiet) {}
  
  enum Verbosity
  {
    verbosityQuiet = 0,
    verbosityProgressAndResult,
    verbosityDetailed,
    verbosityAll
  };

  virtual SolutionContainerPtr createDefaultSolutionContainer(FitnessLimitsPtr limits) const;

  SolutionContainerPtr optimize(ExecutionContext& context, ProblemPtr problem, ObjectPtr initialSolution = ObjectPtr(), Verbosity verbosity = verbosityQuiet);

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution = ObjectPtr(), Verbosity verbosity = verbosityQuiet);
  virtual void optimize(ExecutionContext& context) = 0;
  virtual void clear(ExecutionContext& context);
  
  ProblemPtr getProblem() const
    {return problem;}

  SolutionContainerPtr getSolutions() const
    {return solutions;}

protected:
  typedef std::pair<ObjectPtr, FitnessPtr> SolutionAndFitnessPair;

  ProblemPtr problem;
  SolutionContainerPtr solutions;
  Verbosity verbosity;

  FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
};

extern SolverPtr nrpaSolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel);

class IterativeSolver : public Solver
{
public:
  IterativeSolver(size_t numIterations = 0)
    : numIterations(numIterations) {}

  virtual bool iteration(ExecutionContext& context, size_t iter) = 0; // returns false if the optimizer has converged

  virtual void optimize(ExecutionContext& context);

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
