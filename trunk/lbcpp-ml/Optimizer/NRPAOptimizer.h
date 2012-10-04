/*-----------------------------------------.---------------------------------.
| Filename: NRPAOptimizer.h                | Nested Rollout Policy Adaptation|
| Author  : Francis Maes                   | Optimizer                       |
| Started : 12/09/2012 15:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_NRPA_H_
# define LBCPP_ML_OPTIMIZER_NRPA_H_

# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class NRPAOptimizer : public Optimizer
{
public:
  NRPAOptimizer(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel)
    : sampler(sampler), level(level), numIterationsPerLevel(numIterationsPerLevel) {}
  NRPAOptimizer() : level(0), numIterationsPerLevel(0) {}
  
  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, Verbosity verbosity)
  {
    Optimizer::configure(context, problem, front, verbosity);
    sampler->initialize(context, problem->getDomain());
  }

  virtual void optimize(ExecutionContext& context)
    {optimizeRecursively(context, sampler, level);}

protected:
  friend class NRPAOptimizerClass;

  SamplerPtr sampler;
  size_t level;
  size_t numIterationsPerLevel;

  SolutionAndFitnessPair optimizeRecursively(ExecutionContext& context, SamplerPtr sampler, size_t level)
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
      FitnessPtr bestFitness = problem->getFitnessLimits()->getWorstPossibleFitness(true);
      ObjectPtr bestSolution;

      SamplerPtr currentSampler = sampler->cloneAndCast<Sampler>();
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
        {
          currentSampler->reinforce(context, bestSolution);
          if (currentSampler->isDeterministic())
            break;
        }

        if (isTopLevel && problem->shouldStop())
          break;
      }
      return std::make_pair(bestSolution, bestFitness);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_NRPA_H_
