/*-----------------------------------------.---------------------------------.
| Filename: MOOCore.cpp                    | Multi Objective Optimization    |
| Author  : Francis Maes                   | Base Classes                    |
| Started : 11/09/2012 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "MOOCore.h"
using namespace lbcpp;

/*
** MOOFitnessLimits
*/
MOOFitnessPtr MOOFitnessLimits::getWorstPossibleFitness() const
{
  std::vector<double> res(limits.size());
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = limits[i].first;
  return MOOFitnessPtr(new MOOFitness(res, refCountedPointerFromThis(this)));
}

double MOOFitnessLimits::getObjectiveSign(size_t objectiveIndex) const
  {return limits[objectiveIndex].second > limits[objectiveIndex].first ? 1.0 : -1.0;}

/*
** MOOFitness
*/
MOOFitness::MOOFitness(const std::vector<double>& objectives, const MOOFitnessLimitsPtr& limits)
  : objectives(objectives), limits(limits)
{
}

bool MOOFitness::dominates(const MOOFitnessPtr& other, bool strictly) const
{
  jassert(other->limits == limits);

  size_t numObjectivesForWhichThisIsBetter = 0;
  for (size_t i = 0; i < objectives.size(); ++i)
  {
    double delta = (other->objectives[i] - objectives[i]) * limits->getObjectiveSign(i);
    if (delta > 0)
      return false; // other is better than this
    else if (delta < 0)
      ++numObjectivesForWhichThisIsBetter;
  }
  return !strictly || (numObjectivesForWhichThisIsBetter > 0);
}

bool MOOFitness::strictlyDominates(const MOOFitnessPtr& other) const
  {return dominates(other, true);}

int MOOFitness::compare(const ObjectPtr& otherObject) const
{
  MOOFitnessPtr otherFitness = otherObject.dynamicCast<MOOFitness>();
  if (!otherFitness)
    return Object::compare(otherObject);
  if (limits != otherFitness->limits)
    return limits < otherFitness->limits ? -1 : 1;
  if (objectives != otherFitness->objectives)
    return objectives < otherFitness->objectives ? -1 : 1;
  return 0;
}

/*
** MOOParetoFront
*/
MOOParetoFront::MOOParetoFront(MOOFitnessLimitsPtr limits)
  : limits(limits), size(0)
{
}

MOOParetoFront::MOOParetoFront() : size(0)
{
}

void MOOParetoFront::insert(const ObjectPtr& solution, const MOOFitnessPtr& fitness)
{
  for (ParetoMap::const_iterator it = m.begin(); it != m.end(); ++it)
    if (it->first->strictlyDominates(fitness))
      return; // dominated

  ParetoMap::iterator it = m.find(fitness);
  if (it == m.end())
  {
    ParetoMap::iterator nxt;
    for (it = m.begin(); it != m.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (fitness->strictlyDominates(it->first))
        m.erase(it);
    }
    m[fitness] = std::vector<ObjectPtr>(1, solution);
  }
  else
    it->second.push_back(solution);
  ++size;
}

/*
** MOOOptimizer
*/
MOOParetoFrontPtr MOOOptimizer::optimize(ExecutionContext& context, MOOProblemPtr problem)
{
  MOOParetoFrontPtr res(new MOOParetoFront(problem->getFitnessLimits()));
  optimize(context, problem, res);
  return res;
}
