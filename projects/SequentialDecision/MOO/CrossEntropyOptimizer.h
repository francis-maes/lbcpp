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

// TODO: "Comparator" concept

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
      MOOSolutionSetPtr population = sampleAndEvaluatePopulation(context, sampler, populationSize);
      if (parents)
        population->addSolutions(parents);
      MOOSolutionSetPtr selectedPopulation = population->selectNBests(comparator ? comparator : createDefaultComparator(), numTrainingSamples);

      sampler = sampler->cloneAndCast<MOOSampler>();
      learnSampler(context, selectedPopulation, sampler);

      context.progressCallback(new ProgressionState(i+1, numGenerations, "Generations"));
      if (elitist)
        parents = selectedPopulation;
    }
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
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_
