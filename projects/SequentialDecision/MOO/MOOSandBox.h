/*-----------------------------------------.---------------------------------.
| Filename: MOOSandBox.h                   | Multi Objective Optimization    |
| Author  : Francis Maes                   | SandBox                         |
| Started : 11/09/2012 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SANDBOX_H_
# define LBCPP_MOO_SANDBOX_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class MOOFitness;
typedef ReferenceCountedObjectPtr<MOOFitness> MOOFitnessPtr;

class MOOFitnessLimits;
typedef ReferenceCountedObjectPtr<MOOFitnessLimits> MOOFitnessLimitsPtr;

class MOOProblem;
typedef ReferenceCountedObjectPtr<MOOProblem> MOOProblemPtr;

class MOOParetoSet;
typedef ReferenceCountedObjectPtr<MOOParetoSet> MOOParetoSetPtr;

class MOOOptimizer;
typedef ReferenceCountedObjectPtr<MOOOptimizer> MOOOptimizerPtr;

class MOOSampler;
typedef ReferenceCountedObjectPtr<MOOSampler> MOOSamplerPtr;

///////////////////////

class MOOFitnessLimits : public Object
{
public:
  size_t getNumObjectives() const
    {return limits.size();}

  MOOFitnessPtr getWorstPossibleFitness() const;

  bool shouldObjectiveBeMaximized(size_t objectiveNumber) const
    {return limits[objectiveNumber].second > limits[objectiveNumber].first;}

  bool isFitnessBetter(size_t objectiveNumber, double fitness1, double fitness2) const // returns true if fitness1 is better (or equal to) fitness2
  {
    if (shouldObjectiveBeMaximized(objectiveNumber))
      return fitness1 >= fitness2;
    else
      return fitness1 <= fitness2;
  }

protected:
  friend class MOOFitnessLimitsClass;

  std::vector< std::pair<double, double> > limits; // (worst, best) pairs
};

class MOOFitness : public Object
{
public:
  MOOFitness(const std::vector<double>& objectives, const MOOFitnessLimitsPtr& limits)
    : objectives(objectives), limits(limits) {}
  MOOFitness() {}

  size_t getNumObjectives() const
    {return objectives.size();}

  double getObjective(size_t i) const
    {jassert(i < objectives.size()); return objectives[i];}

  // FIXME: add "strict" flag
  bool dominates(const MOOFitnessPtr& other) const
  {
    jassert(other->limits == limits);

    bool thisIsBetter = false;
    bool theOtherIsBetter = false;
    for (size_t i = 0; i < objectives.size(); ++i)
    {
      if (limits->isFitnessBetter(i, objectives[i], other->objectives[i]))
        thisIsBetter = true;
      else
        theOtherIsBetter = true; // FIXME: support of "strict"
    }
    return thisIsBetter && !theOtherIsBetter;
  }

protected:
  friend class MOOFitnessClass;

  std::vector<double> objectives;
  MOOFitnessLimitsPtr limits;
};

inline MOOFitnessPtr MOOFitnessLimits::getWorstPossibleFitness() const
{
  std::vector<double> res(limits.size());
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = limits[i].first;
  return MOOFitnessPtr(new MOOFitness(res, refCountedPointerFromThis(this)));
}

class MOOParetoSet : public Object
{
public:
  MOOParetoSet(MOOFitnessLimitsPtr limits)
    : limits(limits) {}
  MOOParetoSet() {}

  void insert(const ObjectPtr& solution, const MOOFitnessPtr& fitness)
  {
    jassertfalse;
    // FIXME
  }

  const MOOFitnessLimitsPtr& getLimits() const
    {return limits;}

protected:
  friend class MOOParetoSetClass;

  MOOFitnessLimitsPtr limits;
};

class MOOProblem : public Object
{
public:
  virtual MOOFitnessLimitsPtr getLimits() const = 0;
  virtual MOOFitnessPtr evaluate(const ObjectPtr& solution) = 0;
  virtual bool shouldStop() const
    {return false;}

  size_t getNumObjectives() const
    {return getLimits()->getNumObjectives();}
};

class MOOOptimizer : public Object
{
public:
  MOOParetoSetPtr optimize(ExecutionContext& context, MOOProblemPtr problem)
  {
    MOOParetoSetPtr res(new MOOParetoSet(problem->getLimits()));
    optimize(context, problem, res);
    return res;
  }

  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoSetPtr paretoSet) = 0;

protected:
  typedef std::pair<ObjectPtr, MOOFitnessPtr> SolutionAndFitnessPair;
};

class MOOSampler : public Object
{
public:
  virtual ObjectPtr sample() const = 0;
  virtual void reinforce(const ObjectPtr& solution) = 0;
};

////////////////////////////

class RandomMOOOptimizer : public MOOOptimizer
{
public:
  RandomMOOOptimizer(MOOSamplerPtr sampler, size_t numIterations)
    : sampler(sampler), numIterations(numIterations) {}
  RandomMOOOptimizer() : numIterations(0) {}

  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoSetPtr paretoSet)
  {
    for (size_t iteration = 0; iteration < numIterations && !problem->shouldStop(); ++iteration)
    {
      ObjectPtr solution = sampler->sample();
      paretoSet->insert(solution, problem->evaluate(solution));
    }
  }

protected:
  friend class RandomMOOOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numIterations;
};

class NRPAMOOOptimizer : public MOOOptimizer
{
public:
  NRPAMOOOptimizer(MOOSamplerPtr sampler, size_t level, size_t numIterationsPerLevel)
    : sampler(sampler), level(level), numIterationsPerLevel(numIterationsPerLevel) {}
  NRPAMOOOptimizer() : level(0), numIterationsPerLevel(0) {}
  
  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoSetPtr paretoSet)
    {optimizeRecursively(context, problem, paretoSet, this->sampler->cloneAndCast<MOOSampler>(), level);}

protected:
  friend class NRPAMOOOptimizerClass;

  MOOSamplerPtr sampler;
  size_t level;
  size_t numIterationsPerLevel;

  SolutionAndFitnessPair optimizeRecursively(ExecutionContext& context, MOOProblemPtr problem, MOOParetoSetPtr paretoSet, MOOSamplerPtr sampler, size_t level)
  {
    if (problem->shouldStop())
      return SolutionAndFitnessPair();
      
    if (level == 0)
    {
      ObjectPtr solution = sampler->sample();
      return SolutionAndFitnessPair(solution, problem->evaluate(solution));
    }
    else
    {
      MOOFitnessPtr bestFitness = problem->getLimits()->getWorstPossibleFitness();
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

//////////////////////////////////////

class MOOSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {


    return true;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
