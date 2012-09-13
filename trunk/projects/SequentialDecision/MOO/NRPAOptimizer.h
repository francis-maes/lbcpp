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
    optimizeRecursively(context, this->sampler->cloneAndCast<MOOSampler>(), level);
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
      MOOFitnessPtr bestFitness = problem->getFitnessLimits()->getWorstPossibleFitness();
      ObjectPtr bestSolution;
        
      for (size_t i = 0; i < numIterationsPerLevel; ++i)
      {
        SolutionAndFitnessPair subResult = optimizeRecursively(context, sampler->cloneAndCast<MOOSampler>(), level - 1);
        if (subResult.second->dominates(bestFitness))
        {
          bestSolution = subResult.first;
          bestFitness = subResult.second;
        }
        
        if (bestSolution)
          sampler->reinforce(context, bestSolution);
      }
      return std::make_pair(bestSolution, bestFitness);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_NRPA_H_
