/*-----------------------------------------.---------------------------------.
| Filename: CrossEntropyOptimizer.h        | Cross-Entropy Optimizer         |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_
# define LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_

# include "MOOCore.h"

namespace lbcpp
{

class CrossEntropyOptimizer : public PopulationBasedMOOOptimizer
{
public:
  CrossEntropyOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, MOOSolutionComparatorPtr comparator = MOOSolutionComparatorPtr())
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples), elitist(elitist), comparator(comparator) {}
  CrossEntropyOptimizer() : elitist(false) {}
   
  virtual void optimize(ExecutionContext& context)
  {
    MOOSamplerPtr sampler = this->sampler;
    sampler->initialize(context, problem->getObjectDomain());
    MOOSolutionSetPtr parents;
    for (size_t i = 0; (numGenerations == 0 || i < numGenerations) && !problem->shouldStop(); ++i)
    {
      startGeneration(context, i, sampler);

      MOOSolutionSetPtr population = sampleAndEvaluatePopulation(context, sampler, populationSize);
      if (parents)
        population->addSolutions(parents);
      MOOSolutionSetPtr selectedPopulation = select(population, numTrainingSamples);

      sampler = sampler->cloneAndCast<MOOSampler>();
      learnSampler(context, selectedPopulation, sampler);
      
      if (elitist)
        parents = selectedPopulation;

      finishGeneration(context, i, numGenerations);
    }

    if (verbosity >= verbosityProgressAndResult)
      context.resultCallback("sampler", sampler);
  }

 protected:
  friend class CrossEntropyOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numTrainingSamples;
  bool elitist;
  MOOSolutionComparatorPtr comparator;

  MOOSolutionComparatorPtr createDefaultComparator() const
  {
    if (problem->getNumObjectives() == 1)
      return objectiveComparator(0);  // single-objective
    else
      return paretoRankAndCrowdingDistanceComparator(); // multi-objective
  }
  
  MOOSolutionSetPtr select(const MOOSolutionSetPtr& population, size_t count) const
    {return population->selectNBests(comparator ? comparator : createDefaultComparator(), count);}

  void startGeneration(ExecutionContext& context, size_t index, MOOSamplerPtr sampler)
  {
    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("Generation " + String((int)index + 1));
      context.resultCallback("generation", index + 1);
      context.resultCallback("sampler", sampler->cloneAndCast<MOOSampler>());
    }
  }

  void finishGeneration(ExecutionContext& context, size_t index, size_t numGenerations, bool isTopLevel = true)
  {
    if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("hyperVolume", this->front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
      context.leaveScope();
    }
    if (verbosity >= verbosityProgressAndResult && isTopLevel)
      context.progressCallback(new ProgressionState(index+1, numGenerations, "Generations"));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_
