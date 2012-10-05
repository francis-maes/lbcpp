/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimization Algorithm          |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_H_
# define LBCPP_ML_OPTIMIZER_H_

# include "Problem.h"

namespace lbcpp
{

class Optimizer : public Object
{
public:
  Optimizer() : verbosity(verbosityQuiet) {}
  
  enum Verbosity
  {
    verbosityQuiet = 0,
    verbosityProgressAndResult,
    verbosityDetailed,
    verbosityAll
  };

  ParetoFrontPtr optimize(ExecutionContext& context, ProblemPtr problem, Verbosity verbosity = verbosityQuiet);

  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, Verbosity verbosity = verbosityQuiet);
  virtual void optimize(ExecutionContext& context) = 0;
  virtual void clear(ExecutionContext& context);
  
  ProblemPtr getProblem() const
    {return problem;}

  ParetoFrontPtr getFront() const
    {return front;}

  double computeHyperVolume() const;

protected:
  typedef std::pair<ObjectPtr, FitnessPtr> SolutionAndFitnessPair;

  ProblemPtr problem;
  ParetoFrontPtr front;
  Verbosity verbosity;

  FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object);
  FitnessPtr evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, SolutionSetPtr solutions);
  ObjectPtr sampleSolution(ExecutionContext& context, SamplerPtr sampler);
  FitnessPtr sampleAndEvaluateSolution(ExecutionContext& context, SamplerPtr sampler, SolutionSetPtr solutions = SolutionSetPtr());
  SolutionSetPtr sampleAndEvaluatePopulation(ExecutionContext& context, SamplerPtr sampler, size_t populationSize);
  void learnSampler(ExecutionContext& context, SolutionSetPtr solutions, SamplerPtr sampler);
};

extern OptimizerPtr nrpaOptimizer(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel);

class IterativeOptimizer : public Optimizer
{
public:
  IterativeOptimizer(size_t numIterations = 0)
    : numIterations(numIterations) {}

  virtual bool iteration(ExecutionContext& context, size_t iter) = 0; // returns false if the optimizer has converged

  virtual void optimize(ExecutionContext& context);

protected:
  friend class IterativeOptimizerClass;

  size_t numIterations;
};

extern IterativeOptimizerPtr randomOptimizer(SamplerPtr sampler, size_t numIterations = 0);

class PopulationBasedOptimizer : public IterativeOptimizer
{
public:
  PopulationBasedOptimizer(size_t populationSize = 100, size_t numGenerations = 0)
    : IterativeOptimizer(numGenerations), populationSize(populationSize) {}

protected:
  friend class PopulationBasedOptimizerClass;

  size_t populationSize;
};

extern PopulationBasedOptimizerPtr crossEntropyOptimizer(SamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, SolutionComparatorPtr comparator = SolutionComparatorPtr());
extern PopulationBasedOptimizerPtr nsga2moOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_H_
