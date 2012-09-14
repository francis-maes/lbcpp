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
  CrossEntropyOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples), elitist(elitist) {}
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
      MOOSolutionSetPtr selectedPopulation = selectTrainingSamples(context, population);

      sampler = sampler->cloneAndCast<MOOSampler>();
      learnSampler(context, selectedPopulation, sampler);

      context.progressCallback(new ProgressionState(i+1, numGenerations, "Generations"));
      if (elitist)
        parents = selectedPopulation;
    }
    context.resultCallback("sampler", sampler);
  }

  virtual MOOSolutionSetPtr selectTrainingSamples(ExecutionContext& context, MOOSolutionSetPtr population) const
  {
    // default implementation for single objective
    jassert(problem->getNumObjectives() == 1);
    return population->selectNBests(problem->getFitnessLimits()->makeObjectiveComparator(0), numTrainingSamples);
  }

protected:
  friend class CrossEntropyOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numTrainingSamples;
  bool elitist;
};

class CrossEntropyMOOptimizer : public CrossEntropyOptimizer
{
public:
  CrossEntropyMOOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, bool useCrowding = true)
    : CrossEntropyOptimizer(sampler, populationSize, numTrainingSamples, numGenerations, elitist), useCrowding(useCrowding) {}
  CrossEntropyMOOptimizer() {}

  struct CompareDistance
  {
    CompareDistance(const std::vector<double>& distances)
      : distances(distances) {}

    const std::vector<double>& distances;

    bool operator()(size_t a, size_t b)
      {return distances[a] < distances[b];}
  };

  virtual MOOSolutionSetPtr selectTrainingSamples(ExecutionContext& context, MOOSolutionSetPtr population) const
  {
    jassertfalse;
#if 0
    if (numTrainingSamples >= population->getNumElements())
      return population;

    std::vector<MOOParetoFrontPtr> fronts = population->nonDominatedSort();
    
    MOOSolutionSetPtr res = new MOOSolutionSet();
    for (size_t i = 0; i < fronts.size() && res->getNumElements() < numTrainingSamples; ++i)
    {
      MOOParetoFrontPtr front = fronts[i];
      size_t numRequired = numTrainingSamples - res->getNumElements();
      if (front->getNumElements() <= numRequired)
        // add the whole sub-front
        res->add(front);
      else
      {
        if (useCrowding)
        {
          std::vector<double> distances;
          front->computeCrowdingDistances(distances);
          std::vector<size_t> indices(distances.size());
          for (size_t i = 0; i < indices.size(); ++i)
            indices[i] = i;
          std::sort(indices.begin(), indices.end(), CompareDistance(distances));

          std::vector<MOOFitnessPtr> fitnesses;
          front->getFitnesses(fitnesses);
          for (size_t j = 0; j < indices.size() && res->getNumElements() < numTrainingSamples; ++j)
          {
            MOOFitnessPtr fitness = fitnesses[indices[indices.size() - 1 - j]];
            std::vector<ObjectPtr> solutions;
            front->getSolutionsByFitness(fitness, solutions);
            for (size_t k = 0; k < solutions.size() && res->getNumElements() < numTrainingSamples; ++k)
              res->add(solutions[k], fitness);
          }
        }
        else
        {
          // sample a subset of the sub-front
          std::vector< std::pair<MOOFitnessPtr, ObjectPtr> > solutions;
          front->getSolutionAndFitnesses(solutions);
          std::vector<size_t> order;
          context.getRandomGenerator()->sampleOrder(solutions.size(), order);
          for (size_t j = 0; j < numRequired; ++j)
            res->add(solutions[order[j]].second, solutions[order[j]].first);
        }
      }
    }
    jassert(res->getNumElements() == numTrainingSamples);
    return res;
#endif // 0
    return MOOSolutionSetPtr();
  }

protected:
  friend class CrossEntropyMOOptimizerClass;

  bool useCrowding;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_CROSS_ENTROPY_H_
