/*-----------------------------------------.---------------------------------.
| Filename: Fitness.cpp                    | Fitness                         |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core/Double.h>
#include <ml/Fitness.h>
using namespace lbcpp;

/*
** FitnessLimits
*/
FitnessPtr FitnessLimits::getWorstPossibleFitness(bool useInfiniteValues) const
{
  std::vector<double> res(limits.size());
  if (useInfiniteValues)
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = shouldObjectiveBeMaximised(i) ? -DBL_MAX : DBL_MAX;
  else
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = limits[i].first;
  return FitnessPtr(new Fitness(res, refCountedPointerFromThis(this)));
}

FitnessPtr FitnessLimits::getBestPossibleFitness(bool useInfiniteValues) const
{
  std::vector<double> res(limits.size());
  if (useInfiniteValues)
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = shouldObjectiveBeMaximised(i) ? DBL_MAX : -DBL_MAX;
  else
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = limits[i].second;
  return FitnessPtr(new Fitness(res, refCountedPointerFromThis(this)));
}

double FitnessLimits::computeUtopicHyperVolume() const
{
  double res = 1.0;
  for (size_t i = 0; i < limits.size(); ++i)
    res *= fabs(limits[i].first - limits[i].second);
  return res;
}

bool FitnessLimits::shouldObjectiveBeMaximised(size_t objectiveIndex) const
  {return limits[objectiveIndex].second > limits[objectiveIndex].first;}

double FitnessLimits::getObjectiveSign(size_t objectiveIndex) const
  {return shouldObjectiveBeMaximised(objectiveIndex) ? 1.0 : -1.0;}

/*
** Fitness
*/
Fitness::Fitness(const std::vector<double>& values, const FitnessLimitsPtr& limits)
  : values(values), limits(limits)
{
}

Fitness::Fitness(double value, const FitnessLimitsPtr& limits)
  : values(1, value), limits(limits)
{
}

bool Fitness::dominates(const FitnessPtr& other, bool strictly) const
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

bool Fitness::strictlyDominates(const FitnessPtr& other) const
  {return dominates(other, true);}

bool Fitness::isBetterForAtLeastOneObjectiveThan(const FitnessPtr& other, bool strictly) const
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

double Fitness::multiplicativeEpsilon(const FitnessPtr& other) const
{
  jassert(getNumValues() == other->getNumValues());
  double result = 1.0;
  for (size_t i = 0; i < getNumValues(); ++i)
    if (other->getValue(i) != 0.0)
      result = std::max(result, getValue(i) / other->getValue(i));
  return result;
}

double Fitness::additiveEpsilon(const FitnessPtr& other) const
{
  if (dominates(other, false))
    return 0.0;
  jassert(getNumValues() == other->getNumValues());
  double diff, result = 0.0;
  for (size_t i = 0; i < getNumValues(); ++i)
  {
    diff = getValue(i) - other->getValue(i);
    result += diff * diff;
  }
  result = sqrt(result);
  return result;
}

FitnessPtr Fitness::makeWorstCombination(const FitnessPtr& fitness1, const FitnessPtr& fitness2)
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
  return new Fitness(res, fitness1->limits);
}

int Fitness::compare(const ObjectPtr& otherObject) const
{
  FitnessPtr otherFitness = otherObject.dynamicCast<Fitness>();
  if (!otherFitness)
    return Object::compare(otherObject);
  if (limits != otherFitness->limits)
    return limits < otherFitness->limits ? -1 : 1;
  if (values != otherFitness->values)
    return values < otherFitness->values ? -1 : 1;
  return 0;
}

string Fitness::toShortString() const
{
  string res = "(";
  for (size_t i = 0; i < values.size(); ++i)
  {
    res += ObjectPtr(new Double(values[i]))->toShortString();
    if (i < values.size() - 1)
      res += ", ";
  }
  res += ")";
  return res;
}

double Fitness::toDouble() const
{
  double res = 0.0;
  for (size_t i = 0; i < values.size(); ++i)
    res += values[i];
  return res;
}

std::vector<double> Fitness::getValuesToBeMinimized() const
{
  std::vector<double> res = getValues();
  for (size_t i = 0; i < res.size(); ++i)
    if (limits->shouldObjectiveBeMaximised(i))
      res[i] = -res[i];
  return res;
}
