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
# include <lbcpp-ml/SolutionComparator.h>

namespace lbcpp
{

class CrossEntropyOptimizer : public PopulationBasedOptimizer
{
public:
  CrossEntropyOptimizer(SamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, SolutionComparatorPtr comparator = SolutionComparatorPtr())
    : PopulationBasedOptimizer(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples), elitist(elitist), comparator(comparator) {}
  CrossEntropyOptimizer() : elitist(false) {}
  
  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeOptimizer::configure(context, problem, front, initialSolution, verbosity);
    currentSampler = this->sampler;
    currentSampler->initialize(context, problem->getDomain());
    currentParents = SolutionSetPtr();
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    if (verbosity >= verbosityDetailed)
      context.resultCallback("currentSampler", currentSampler->cloneAndCast<Sampler>());

    SolutionSetPtr population = sampleAndEvaluatePopulation(context, currentSampler, populationSize);
    if (currentParents)
      population->addSolutions(currentParents);
    SolutionSetPtr selectedPopulation = select(population, numTrainingSamples);

    currentSampler = currentSampler->cloneAndCast<Sampler>();
    learnSampler(context, selectedPopulation, currentSampler);
    
    if (elitist)
      currentParents = selectedPopulation;

    return !currentSampler->isDeterministic();
  }

  virtual void clear(ExecutionContext& context)
  {
    if (verbosity >= verbosityProgressAndResult)
      context.resultCallback("sampler", currentSampler);
    Optimizer::clear(context);
  }

 protected:
  friend class CrossEntropyOptimizerClass;

  SamplerPtr sampler;
  size_t numTrainingSamples;
  bool elitist;
  SolutionComparatorPtr comparator;

  SamplerPtr currentSampler;
  SolutionSetPtr currentParents;

  SolutionComparatorPtr createDefaultComparator() const
  {
    if (problem->getNumObjectives() == 1)
      return objectiveComparator(0);  // single-objective
    else
      return paretoRankAndCrowdingDistanceComparator(); // multi-objective
  }
  
  SolutionSetPtr select(const SolutionSetPtr& population, size_t count) const
    {return population->selectNBests(comparator ? comparator : createDefaultComparator(), count);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_CROSS_ENTROPY_H_
