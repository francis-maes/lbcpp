/*-----------------------------------------.---------------------------------.
| Filename: CrossEntropyOptimizer.h        | Cross-Entropy Optimizer         |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_CROSS_ENTROPY_H_
# define LBCPP_ML_OPTIMIZER_CROSS_ENTROPY_H_

# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionSet.h>
# include <lbcpp-ml/Comparator.h>

namespace lbcpp
{

class CrossEntropyOptimizer : public PopulationBasedMOOOptimizer
{
public:
  CrossEntropyOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, MOOSolutionComparatorPtr comparator = MOOSolutionComparatorPtr())
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples), elitist(elitist), comparator(comparator) {}
  CrossEntropyOptimizer() : elitist(false) {}
  
  virtual void configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity)
  {
    IterativeOptimizer::configure(context, problem, front, verbosity);
    currentSampler = this->sampler;
    currentSampler->initialize(context, problem->getObjectDomain());
    currentParents = MOOSolutionSetPtr();
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    if (verbosity >= verbosityDetailed)
      context.resultCallback("currentSampler", currentSampler->cloneAndCast<MOOSampler>());

    MOOSolutionSetPtr population = sampleAndEvaluatePopulation(context, currentSampler, populationSize);
    if (currentParents)
      population->addSolutions(currentParents);
    MOOSolutionSetPtr selectedPopulation = select(population, numTrainingSamples);

    currentSampler = currentSampler->cloneAndCast<MOOSampler>();
    learnSampler(context, selectedPopulation, currentSampler);
    
    if (elitist)
      currentParents = selectedPopulation;

    return !currentSampler->isDegenerate();
  }

  virtual void clear(ExecutionContext& context)
  {
    if (verbosity >= verbosityProgressAndResult)
      context.resultCallback("sampler", currentSampler);
    MOOOptimizer::clear(context);
  }

 protected:
  friend class CrossEntropyOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numTrainingSamples;
  bool elitist;
  MOOSolutionComparatorPtr comparator;

  MOOSamplerPtr currentSampler;
  MOOSolutionSetPtr currentParents;

  MOOSolutionComparatorPtr createDefaultComparator() const
  {
    if (problem->getNumObjectives() == 1)
      return objectiveComparator(0);  // single-objective
    else
      return paretoRankAndCrowdingDistanceComparator(); // multi-objective
  }
  
  MOOSolutionSetPtr select(const MOOSolutionSetPtr& population, size_t count) const
    {return population->selectNBests(comparator ? comparator : createDefaultComparator(), count);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_CROSS_ENTROPY_H_
