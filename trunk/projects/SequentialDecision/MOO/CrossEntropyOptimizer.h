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
  CrossEntropyOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples) {}
  CrossEntropyOptimizer() {}

  virtual void optimize(ExecutionContext& context)
  {
    MOOSamplerPtr sampler = this->sampler;
    sampler->initialize(context, problem->getSolutionDomain());
    for (size_t i = 0; (numGenerations == 0 || i < numGenerations) && !problem->shouldStop(); ++i)
    {
      MOOSolutionSetPtr population = sampleAndEvaluatePopulation(context, sampler, populationSize);
      MOOSolutionSetPtr selectedPopulation = selectTrainingSamples(context, population);

      sampler = sampler->cloneAndCast<MOOSampler>();
      learnSampler(context, selectedPopulation, sampler);

      context.progressCallback(new ProgressionState(i+1, numGenerations, "Generations"));
    }
    context.resultCallback("sampler", sampler);
  }

  virtual MOOSolutionSetPtr selectTrainingSamples(ExecutionContext& context, MOOSolutionSetPtr population) const
  {
    // default implementation for single objective
    jassert(problem->getNumObjectives() == 1);

    std::vector< std::pair<MOOFitnessPtr, ObjectPtr> > pop;
    population->getSolutionAndFitnesses(pop);
    bool isMaximisation = problem->getFitnessLimits()->shouldObjectiveBeMaximized(0);

    size_t size = numTrainingSamples < pop.size() ? numTrainingSamples : pop.size();
    MOOSolutionSetPtr res = new MOOSolutionSet();
    for (size_t i = 0; i < size; ++i)
    {
      size_t index = (isMaximisation ? pop.size() - 1 - i : i);
      res->add(pop[index].second, pop[index].first);
    }
    return res;
  }

protected:
  friend class CrossEntropyOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numTrainingSamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_
