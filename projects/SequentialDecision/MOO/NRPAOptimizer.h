/*-----------------------------------------.---------------------------------.
| Filename: NRPAOptimizer.h                | Nested Rollout Policy Adaptation|
| Author  : Francis Maes                   | Optimizer                       |
| Started : 12/09/2012 15:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_OPTIMIZER_NRPA_H_
# define LBCPP_MOO_OPTIMIZER_NRPA_H_

# include "MOOCore.h"

namespace lbcpp
{

class NRPAOptimizer : public MOOOptimizer
{
public:
  NRPAOptimizer(MOOSamplerPtr sampler, size_t level, size_t numIterationsPerLevel)
    : sampler(sampler), level(level), numIterationsPerLevel(numIterationsPerLevel) {}
  NRPAOptimizer() : level(0), numIterationsPerLevel(0) {}
  
  virtual void optimize(ExecutionContext& context)
  {
    sampler->initialize(context, problem->getSolutionDomain());
    optimizeRecursively(context, sampler, level);
  }

protected:
  friend class NRPAOptimizerClass;

  MOOSamplerPtr sampler;
  size_t level;
  size_t numIterationsPerLevel;

  SolutionAndFitnessPair optimizeRecursively(ExecutionContext& context, MOOSamplerPtr sampler, size_t level)
  {
    if (problem->shouldStop())
      return SolutionAndFitnessPair();
      
    if (level == 0)
    {
      ObjectPtr solution = sampler->sample(context);
      return SolutionAndFitnessPair(solution, evaluate(context, solution));
    }
    else
    {
      MOOFitnessPtr bestFitness = problem->getFitnessLimits()->getWorstPossibleFitness(true);
      ObjectPtr bestSolution;

      MOOSamplerPtr currentSampler = sampler->cloneAndCast<MOOSampler>();
      bool isTopLevel = (this->level == level);
      for (size_t i = 0; isTopLevel || i < numIterationsPerLevel; ++i)
      {
        SolutionAndFitnessPair subResult = optimizeRecursively(context, currentSampler, level - 1);
        if (subResult.second && subResult.second->dominates(bestFitness))
        {
          bestSolution = subResult.first;
          bestFitness = subResult.second;
        }
        
        if (bestSolution)
          currentSampler->reinforce(context, bestSolution);

        if (isTopLevel && problem->shouldStop())
          break;
      }
      return std::make_pair(bestSolution, bestFitness);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_NRPA_H_
