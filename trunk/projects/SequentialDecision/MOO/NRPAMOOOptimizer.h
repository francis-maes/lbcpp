/*-----------------------------------------.---------------------------------.
| Filename: NRPAMOOOptimizer.h             | Nested Rollout Policy Adaptation|
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

class NRPAMOOOptimizer : public MOOOptimizer
{
public:
  NRPAMOOOptimizer(MOOSamplerPtr sampler, size_t level, size_t numIterationsPerLevel)
    : sampler(sampler), level(level), numIterationsPerLevel(numIterationsPerLevel) {}
  NRPAMOOOptimizer() : level(0), numIterationsPerLevel(0) {}
  
  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoSet)
    {optimizeRecursively(context, problem, paretoSet, this->sampler->cloneAndCast<MOOSampler>(), level);}

protected:
  friend class NRPAMOOOptimizerClass;

  MOOSamplerPtr sampler;
  size_t level;
  size_t numIterationsPerLevel;

  SolutionAndFitnessPair optimizeRecursively(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoSet, MOOSamplerPtr sampler, size_t level)
  {
    if (problem->shouldStop())
      return SolutionAndFitnessPair();
      
    if (level == 0)
    {
      ObjectPtr solution = sampler->sample(context, problem->getSolutionDomain());
      return SolutionAndFitnessPair(solution, problem->evaluate(context, solution));
    }
    else
    {
      MOOFitnessPtr bestFitness = problem->getFitnessLimits()->getWorstPossibleFitness();
      ObjectPtr bestSolution;
        
      for (size_t i = 0; i < numIterationsPerLevel; ++i)
      {
        SolutionAndFitnessPair subResult = optimizeRecursively(context, problem, paretoSet, sampler->cloneAndCast<MOOSampler>(), level - 1);
        if (subResult.second->dominates(bestFitness))
        {
          bestSolution = subResult.first;
          bestFitness = subResult.second;
        }
        
        if (bestSolution)
          sampler->reinforce(bestSolution);
      }
      return std::make_pair(bestSolution, bestFitness);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_NRPA_H_
