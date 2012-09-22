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

class MOOOptimizer : public Object
{
public:
  MOOOptimizer() : verbosity(verbosityQuiet) {}
  
  enum Verbosity
  {
    verbosityQuiet = 0,
    verbosityProgressAndResult,
    verbosityDetailed,
    verbosityAll
  };

  MOOParetoFrontPtr optimize(ExecutionContext& context, MOOProblemPtr problem, Verbosity verbosity = verbosityQuiet);

  virtual void configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity = verbosityQuiet);
  virtual void optimize(ExecutionContext& context) = 0;
  virtual void clear(ExecutionContext& context);
  
  MOOProblemPtr getProblem() const
    {return problem;}

  MOOParetoFrontPtr getFront() const
    {return front;}

  double computeHyperVolume() const;

protected:
  typedef std::pair<ObjectPtr, MOOFitnessPtr> SolutionAndFitnessPair;

  MOOProblemPtr problem;
  MOOParetoFrontPtr front;
  Verbosity verbosity;

  MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object);
  MOOFitnessPtr evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, MOOSolutionSetPtr solutions);
  ObjectPtr sampleSolution(ExecutionContext& context, MOOSamplerPtr sampler);
  MOOFitnessPtr sampleAndEvaluateSolution(ExecutionContext& context, MOOSamplerPtr sampler, MOOSolutionSetPtr solutions = MOOSolutionSetPtr());
  MOOSolutionSetPtr sampleAndEvaluatePopulation(ExecutionContext& context, MOOSamplerPtr sampler, size_t populationSize);
  void learnSampler(ExecutionContext& context, MOOSolutionSetPtr solutions, MOOSamplerPtr sampler);
};

class IterativeOptimizer : public MOOOptimizer
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

extern IterativeOptimizerPtr randomOptimizer(MOOSamplerPtr sampler, size_t numIterations = 0);

class PopulationBasedMOOOptimizer : public IterativeOptimizer
{
public:
  PopulationBasedMOOOptimizer(size_t populationSize = 100, size_t numGenerations = 0)
    : IterativeOptimizer(numGenerations), populationSize(populationSize) {}

protected:
  friend class PopulationBasedMOOOptimizerClass;

  size_t populationSize;
};

extern PopulationBasedMOOOptimizerPtr crossEntropyOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, MOOSolutionComparatorPtr comparator = MOOSolutionComparatorPtr());

extern PopulationBasedMOOOptimizerPtr nsga2moOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_H_
