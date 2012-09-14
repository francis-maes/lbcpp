/*-----------------------------------------.---------------------------------.
| Filename: CrossEntropyOptimizer.h        | Cross-Entropy Optimizer         |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_OPTIMIZER_NESTED_CROSS_ENTROPY_H_
# define LBCPP_MOO_OPTIMIZER_NESTED_CROSS_ENTROPY_H_

# include "CrossEntropyOptimizer.h"

namespace lbcpp
{

class NestedCrossEntropyOptimizer : public CrossEntropyOptimizer
{
public:
  NestedCrossEntropyOptimizer(MOOSamplerPtr sampler, size_t level, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 10, bool elitist = true)
    : CrossEntropyOptimizer(sampler, populationSize, numTrainingSamples, numGenerations, elitist), level(level) {}
  NestedCrossEntropyOptimizer() {}
 
  virtual void optimize(ExecutionContext& context)
  {
    sampler->initialize(context, problem->getObjectDomain());
    optimizeRecursively(context, sampler, level);
    context.resultCallback("sampler", sampler);
  }

  MOOSolutionSetPtr optimizeRecursively(ExecutionContext& context, MOOSamplerPtr sampler, size_t level)
  {
    sampler->initialize(context, problem->getObjectDomain());
    MOOSolutionSetPtr lastPopulation;

    bool isTopLevel = (level == this->level);
    for (size_t i = 0; (isTopLevel || i < numGenerations) && !problem->shouldStop(); ++i)
    {
      startGeneration(context, i, sampler);

      MOOSolutionSetPtr population = new MOOSolutionSet(problem->getFitnessLimits());
      for (size_t j = 0; j < populationSize; ++j)
      {
        if (level == 0)
        {
          ObjectPtr object = sampleSolution(context, sampler); 
          population->addSolution(object, evaluate(context, object));
        }
        else
        {
          if (verbosity >= verbosityAll)
            context.enterScope("Sub-optimize " + String((int)j+1));
          MOOSolutionSetPtr subSolutions = optimizeRecursively(context, sampler, level - 1);
          if (subSolutions)
            population->addSolutions(subSolutions->getParetoFront());
          if (verbosity >= verbosityAll)
            context.leaveScope();
        }
      }
      
      if (elitist && lastPopulation)
        population->addSolutions(lastPopulation);
      MOOSolutionSetPtr selectedPopulation = select(population, numTrainingSamples);

      sampler = sampler->cloneAndCast<MOOSampler>();
      learnSampler(context, selectedPopulation, sampler);
      
      lastPopulation = selectedPopulation;

      finishGeneration(context, i, numGenerations);
    }

    if (verbosity >= verbosityProgressAndResult)
      context.resultCallback("sampler", sampler);
    return lastPopulation;
  }

  /*
  MOOSolutionSetPtr optimizeRecursively(ExecutionContext& context, MOOSamplerPtr sampler, size_t level)
  {
    if (problem->shouldStop())
      return MOOSolutionSetPtr();
    
    if (level == 0)
    {
      MOOOptimizerPtr crossEntropy = new CrossEntropyOptimizer(sampler, populationSize, numTrainingSamples, numGenerations, elitist, comparator);
      return crossEntropy->optimize(context, problem);
    }
    else
    {
      MOOSamplerPtr currentSampler = sampler->cloneAndCast<MOOSampler>();

      bool isTopLevel = (this->level == level);

      MOOSolutionSetPtr population = new MOOSolutionSet(problem->getFitnessLimits());
      for (size_t i = 0; isTopLevel || i < numGenerations; ++i)
      {
        startGeneration(context, i, currentSampler);

        for (size_t j = 0; j < populationSize; ++j)
        {

        }

        
        MOOSolutionSetPtr subPopulation = optimizeRecursively(context, currentSampler, level - 1);
        if (subPopulation)
        {
          jassert(subPopulation->getNumSolutions() <= numTrainingSamples);
          if (verbosity >= verbosityAll)
            context.resultCallback("solutions", subPopulation);

          population->addSolutions(subPopulation);
          population = select(population, numTrainingSamples);
          learnSampler(context, elitist ? population : subPopulation, currentSampler);
        }
        finishGeneration(context, i, isTopLevel ? 0 : populationSize, isTopLevel);
        if (problem->shouldStop())
          break;
      }
      return population;
    }
  }
  */
protected:
  friend class NestedCrossEntropyOptimizerClass;

  size_t level;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_NESTED_CROSS_ENTROPY_H_
