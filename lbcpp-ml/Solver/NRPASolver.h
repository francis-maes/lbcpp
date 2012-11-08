/*-----------------------------------------.---------------------------------.
| Filename: NRPASolver.h                   | Nested Rollout Policy Adaptation|
| Author  : Francis Maes                   | Solver                          |
| Started : 12/09/2012 14:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_NRPA_H_
# define LBCPP_ML_SOLVER_NRPA_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>

namespace lbcpp
{

class NRPASolver : public Solver
{
public:
  NRPASolver(SamplerPtr sampler, size_t level, size_t numIterationsPerLevel)
    : sampler(sampler), level(level), numIterationsPerLevel(numIterationsPerLevel) {}
  NRPASolver() : level(0), numIterationsPerLevel(0) {}
  
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    Solver::startSolver(context, problem, callback, startingSolution);
    sampler->initialize(context, problem->getDomain());
  }

  virtual void runSolver(ExecutionContext& context)
    {solveRecursively(context, sampler, level);}

protected:
  friend class NRPASolverClass;

  SamplerPtr sampler;
  size_t level;
  size_t numIterationsPerLevel;

  SolutionAndFitnessPair solveRecursively(ExecutionContext& context, SamplerPtr sampler, size_t level)
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
        SolutionAndFitnessPair subResult = solveRecursively(context, currentSampler, level - 1);
        if (subResult.second && subResult.second->dominates(bestFitness))
        {
          bestSolution = subResult.first;
          bestFitness = subResult.second;
        }
        
        if (bestSolution)
        {
          currentSampler->reinforce(context, bestSolution, 1.0);
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

#endif // !LBCPP_ML_SOLVER_NRPA_H_
