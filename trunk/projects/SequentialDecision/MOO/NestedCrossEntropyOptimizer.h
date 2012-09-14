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
  NestedCrossEntropyOptimizer(MOOSamplerPtr sampler, size_t level, size_t populationSize, size_t numTrainingSamples)
    : CrossEntropyOptimizer(sampler, populationSize, numTrainingSamples, 0), level(level) {}
  NestedCrossEntropyOptimizer() {}
 
  virtual void optimize(ExecutionContext& context)
  {
    sampler->initialize(context, problem->getObjectDomain());
    optimizeRecursively(context, sampler, level);
    context.resultCallback("sampler", sampler);
  }

  MOOSolutionSetPtr optimizeRecursively(ExecutionContext& context, MOOSamplerPtr sampler, size_t level)
  {
    MOOSolutionSetPtr population;
    if (problem->shouldStop())
      return population;
    
    if (level == 0)
      population = sampleAndEvaluatePopulation(context, sampler, populationSize);
    else
    {
      MOOSamplerPtr currentSampler = sampler->cloneAndCast<MOOSampler>();
      bool isTopLevel = (this->level == level);

      population = new MOOSolutionSet(problem->getFitnessLimits());
      for (size_t i = 0; isTopLevel || i < populationSize; ++i)
      {
        MOOSolutionSetPtr subSolutions = optimizeRecursively(context, currentSampler, level - 1);
        if (subSolutions)
        {
          population->addSolutions(subSolutions);
          learnSampler(context, subSolutions, currentSampler);
          //learnSampler(context, selectTrainingSamples(context, population), currentSampler);
        }
        if (isTopLevel && problem->shouldStop())
          break;
      }
    }
    return selectTrainingSamples(context, population);
  }

protected:
  friend class NestedCrossEntropyOptimizerClass;

  size_t level;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_NESTED_CROSS_ENTROPY_H_
