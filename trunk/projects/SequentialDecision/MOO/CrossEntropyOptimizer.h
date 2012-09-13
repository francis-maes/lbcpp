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
      MOOParetoFrontPtr population = sampleAndEvaluatePopulation(context, sampler, populationSize);
      std::vector<ObjectPtr> samples = selectTrainingSamples(context, population);
      sampler = sampler->cloneAndCast<MOOSampler>();
      sampler->learn(context, samples);
      context.progressCallback(new ProgressionState(i+1, numGenerations, "Generations"));
    }
    context.resultCallback("sampler", sampler);
  }

  virtual std::vector<ObjectPtr> selectTrainingSamples(ExecutionContext& context, MOOParetoFrontPtr population) const
  {
    // default implementation for single objective
    jassert(problem->getNumObjectives() == 1);

    std::vector< std::pair<MOOFitnessPtr, ObjectPtr> > pop;
    population->getSolutions(pop);
    bool isMaximisation = problem->getFitnessLimits()->shouldObjectiveBeMaximized(0);

    std::vector<ObjectPtr> res(numTrainingSamples < pop.size() ? numTrainingSamples : pop.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = pop[isMaximisation ? res.size() - 1 - i : i].second;
    return res;
  }

protected:
  friend class CrossEntropyOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numTrainingSamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_
