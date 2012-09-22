/*-----------------------------------------.---------------------------------.
| Filename: Fitness.cpp                    | Fitness                         |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Fitness.h>
using namespace lbcpp;

/*
** MOOFitnessLimits
*/
MOOFitnessPtr MOOFitnessLimits::getWorstPossibleFitness(bool useInfiniteValues) const
{
  std::vector<double> res(limits.size());
  if (useInfiniteValues)
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = shouldObjectiveBeMaximised(i) ? -DBL_MAX : DBL_MAX;
  else
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = limits[i].first;
  return MOOFitnessPtr(new MOOFitness(res, refCountedPointerFromThis(this)));
}

MOOFitnessPtr MOOFitnessLimits::getBestPossibleFitness(bool useInfiniteValues) const
{
  std::vector<double> res(limits.size());
  if (useInfiniteValues)
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = shouldObjectiveBeMaximised(i) ? DBL_MAX : -DBL_MAX;
  else
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = limits[i].second;
  return MOOFitnessPtr(new MOOFitness(res, refCountedPointerFromThis(this)));
}

double MOOFitnessLimits::computeUtopicHyperVolume() const
{
  double res = 1.0;
  for (size_t i = 0; i < limits.size(); ++i)
    res *= fabs(limits[i].first - limits[i].second);
  return res;
}

bool MOOFitnessLimits::shouldObjectiveBeMaximised(size_t objectiveIndex) const
  {return limits[objectiveIndex].second > limits[objectiveIndex].first;}

double MOOFitnessLimits::getObjectiveSign(size_t objectiveIndex) const
  {return shouldObjectiveBeMaximised(objectiveIndex) ? 1.0 : -1.0;}

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
    if (limits->shouldObjectiveBeMaximised(i))
      res[i] = -res[i];
  return res;
}
