/*-----------------------------------------.---------------------------------.
| Filename: MOOCore.cpp                    | Multi Objective Optimization    |
| Author  : Francis Maes                   | Base Classes                    |
| Started : 11/09/2012 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "MOOCore.h"
#include <MOO-EALib/Hypervolume.h>
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

bool MOOFitnessLimits::shouldObjectiveBeMaximized(size_t objectiveIndex) const
  {return limits[objectiveIndex].second > limits[objectiveIndex].first;}

double MOOFitnessLimits::getObjectiveSign(size_t objectiveIndex) const
  {return shouldObjectiveBeMaximized(objectiveIndex) ? 1.0 : -1.0;}

/*
** MOOFitness
*/
MOOFitness::MOOFitness(const std::vector<double>& values, const MOOFitnessLimitsPtr& limits)
  : values(values), limits(limits)
{
}

bool MOOFitness::dominates(const MOOFitnessPtr& other, bool strictly) const
{
  jassert(other->limits == limits);

  size_t numObjectivesForWhichThisIsBetter = 0;
  for (size_t i = 0; i < values.size(); ++i)
  {
    double delta = (other->values[i] - values[i]) * limits->getObjectiveSign(i);
    if (delta > 0)
      return false; // other is better than this
    else if (delta < 0)
      ++numObjectivesForWhichThisIsBetter;
  }
  return !strictly || (numObjectivesForWhichThisIsBetter > 0);
}

bool MOOFitness::strictlyDominates(const MOOFitnessPtr& other) const
  {return dominates(other, true);}

bool MOOFitness::isBetterForAtLeastOneObjectiveThan(const MOOFitnessPtr& other, bool strictly) const
{
  jassert(other->limits == limits);
  for (size_t i = 0; i < values.size(); ++i)
  {
    double delta = (other->values[i] - values[i]) * limits->getObjectiveSign(i);
    if ((strictly && delta < 0) || (!strictly && delta <= 0))
      return true;
  }
  return false;
}

MOOFitnessPtr MOOFitness::makeWorstCombination(const MOOFitnessPtr& fitness1, const MOOFitnessPtr& fitness2)
{
  jassert(fitness1->limits == fitness2->limits);
  size_t n = fitness1->values.size();
  std::vector<double> res(n);
  for (size_t i = 0; i < n; ++i)
  {
    double objective1 = fitness1->getValue(i);
    double objective2 = fitness2->getValue(i);
    if (fitness1->limits->getObjectiveSign(i) * (objective2 - objective1) > 0)
      res[i] = objective1;
    else
      res[i] = objective2;
  }
  return new MOOFitness(res, fitness1->limits);
}

int MOOFitness::compare(const ObjectPtr& otherObject) const
{
  MOOFitnessPtr otherFitness = otherObject.dynamicCast<MOOFitness>();
  if (!otherFitness)
    return Object::compare(otherObject);
  if (limits != otherFitness->limits)
    return limits < otherFitness->limits ? -1 : 1;
  if (values != otherFitness->values)
    return values < otherFitness->values ? -1 : 1;
  return 0;
}

String MOOFitness::toShortString() const
{
  String res = "(";
  for (size_t i = 0; i < values.size(); ++i)
  {
    res += Variable(values[i]).toShortString();
    if (i < values.size() - 1)
      res += ", ";
  }
  res += ")";
  return res;
}

std::vector<double> MOOFitness::getValuesToBeMinimized() const
{
  std::vector<double> res = getValues();
  for (size_t i = 0; i < res.size(); ++i)
    if (limits->shouldObjectiveBeMaximized(i))
      res[i] = -res[i];
  return res;
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

void MOOParetoFront::getSolutions(std::vector< std::pair<MOOFitnessPtr, ObjectPtr> >& res) const
{
  res.reserve(size);
  for (ParetoMap::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    MOOFitnessPtr fitness = it->first;
    const std::vector<ObjectPtr>& solutions = it->second;
    for (size_t i = 0; i < solutions.size(); ++i)
      res.push_back(std::make_pair(fitness, solutions[i]));
  }
}

void MOOParetoFront::getSolutionsByFitness(const MOOFitnessPtr& fitness, std::vector<ObjectPtr>& res) const
{
  ParetoMap::const_iterator it = m.find(fitness);
  if (it == m.end())
    res.clear();
  else
    res = it->second;
}

MOOFitnessLimitsPtr MOOParetoFront::getEmpiricalLimits() const
{
  size_t n = limits->getNumDimensions();
  std::vector< std::pair<double, double> > res(n, std::make_pair(DBL_MAX, -DBL_MAX));

  for (ParetoMap::const_iterator it = m.begin(); it != m.end(); ++it)
    for (size_t i = 0; i < n; ++i)
    {
      double value = it->first->getValue(i);
      if (value < res[i].first)
        res[i].first = value;
      if (value > res[i].second)
        res[i].second = value;
    }

  return new MOOFitnessLimits(res);
}

double MOOParetoFront::computeHyperVolume(const MOOFitnessPtr& referenceFitness) const
{
  if (m.empty())
    return 0.0;

  size_t numObjectives = limits->getNumObjectives();
  if (numObjectives == 1)
  {
    double bestValue = (limits->shouldObjectiveBeMaximized(0) ? m.rbegin()->first->getValue(0) : m.begin()->first->getValue(0));
    double res = limits->getObjectiveSign(0) * (bestValue - referenceFitness->getValue(0));
    return res > 0.0 ? res : 0.0;
  }
  else
  {
    // remove points that fall outside the hypervolume and convert to minimization problem
    std::vector<double> points(m.size() * numObjectives);
    size_t numPoints = 0;
    for (ParetoMap::const_iterator it = m.begin(); it != m.end(); ++it)
      if (!referenceFitness->isBetterForAtLeastOneObjectiveThan(it->first))
      {
        std::vector<double> val = it->first->getValuesToBeMinimized();
        memcpy(&points[numPoints * numObjectives], &val[0], sizeof (double) * numObjectives);
        ++numPoints;
      }

    if (!numPoints)
      return 0.0;

    std::vector<double> ref = referenceFitness->getValuesToBeMinimized();

    // shark implementation
    return hypervolume(&points[0], &ref[0], (unsigned int)numObjectives, (unsigned int)numPoints);
  }
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
