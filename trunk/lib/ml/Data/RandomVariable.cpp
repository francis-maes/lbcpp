/*-----------------------------------------.---------------------------------.
| Filename: RandomVariable.cpp             | Random variable statistics      |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <ml/RandomVariable.h>
#include <oil/Core.h>
#include <oil/Lua/Lua.h>
using namespace lbcpp;

/*
** ScalarVariableMean
*/
ScalarVariableMean::ScalarVariableMean(const string& name)
  : NameableObject(name), samplesSum(0.0), samplesCount(0.0)
{
}

string ScalarVariableMean::toString() const
{
  string res;
  string name = getName();
  if (name.isNotEmpty())
    res = name + T(" = ");
  res += string(getMean());
  return res;
}

string ScalarVariableMean::toShortString() const
  {return ObjectPtr(new Double(getMean()))->toShortString();}

int ScalarVariableMean::clear(LuaState& state)
{
  ScalarVariableMeanPtr stats = state.checkObject(1, scalarVariableMeanClass).staticCast<ScalarVariableMean>();
  stats->clear();
  return 0;
}

int ScalarVariableMean::observe(LuaState& state)
{
  ScalarVariableMeanPtr stats = state.checkObject(1, scalarVariableMeanClass).staticCast<ScalarVariableMean>();
  double value = state.checkNumber(2);
  if (state.getTop() >= 3)
  {
    double weight = state.checkNumber(3);
    stats->push(value, weight);
  }
  else
    stats->push(value);
  return 0;    
}

int ScalarVariableMean::getMean(LuaState& state)
{
  ScalarVariableMeanPtr stats = state.checkObject(1, scalarVariableMeanClass).staticCast<ScalarVariableMean>();
  state.pushNumber(stats->getMean());
  return 1;
}

int ScalarVariableMean::getSum(LuaState& state)
{
  ScalarVariableMeanPtr stats = state.checkObject(1, scalarVariableMeanClass).staticCast<ScalarVariableMean>();
  state.pushNumber(stats->getSum());
  return 1;
}

int ScalarVariableMean::getCount(LuaState& state)
{
  ScalarVariableMeanPtr stats = state.checkObject(1, scalarVariableMeanClass).staticCast<ScalarVariableMean>();
  state.pushNumber(stats->getCount());
  return 1;
}


/*
** ScalarVariableMeanAndVariance
*/
ScalarVariableMeanAndVariance::ScalarVariableMeanAndVariance(const string& name)
  : ScalarVariableMean(name), samplesSumOfSquares(0.0) {}

string ScalarVariableMeanAndVariance::toString() const
  {return ScalarVariableMean::toString() + " +/- " + string(getStandardDeviation());}

string ScalarVariableMeanAndVariance::toShortString() const
{
  string res = ScalarVariableMean::toShortString();
  double stddev = getStandardDeviation();
  if (stddev)
    res += T(" +/- ") + ObjectPtr(new Double(stddev))->toShortString();
  return res;
}

int ScalarVariableMeanAndVariance::getStandardDeviation(LuaState& state)
{
  ScalarVariableMeanAndVariancePtr stats = state.checkObject(1, scalarVariableMeanAndVarianceClass).staticCast<ScalarVariableMeanAndVariance>();
  state.pushNumber(stats->getStandardDeviation());
  return 1;
}

int ScalarVariableMeanAndVariance::getVariance(LuaState& state)
{
  ScalarVariableMeanAndVariancePtr stats = state.checkObject(1, scalarVariableMeanAndVarianceClass).staticCast<ScalarVariableMeanAndVariance>();
  state.pushNumber(stats->getVariance());
  return 1;
}

/*
** ScalarVariableStatistics
*/
ScalarVariableStatistics::ScalarVariableStatistics(const string& name)
  : ScalarVariableMeanAndVariance(name), minimumValue(DBL_MAX), maximumValue(-DBL_MAX)
{
}

void ScalarVariableStatistics::clear()
{
  ScalarVariableMeanAndVariance::clear();
  minimumValue = DBL_MAX;
  maximumValue = -DBL_MAX;
}

void ScalarVariableStatistics::push(double val)
{
  ScalarVariableMeanAndVariance::push(val);
  if (val < minimumValue)
    minimumValue = val;
  if (val > maximumValue)
    maximumValue = val;
}

void ScalarVariableStatistics::push(const std::vector<double>& values)
{
  for (size_t i = 0; i < values.size(); ++i)
    push(values[i]);
}

void ScalarVariableStatistics::push(double val, double weight)
{
  ScalarVariableMeanAndVariance::push(val, weight);
  if (val < minimumValue)
    minimumValue = val;
  if (val > maximumValue)
    maximumValue = val;
}

void ScalarVariableStatistics::push(const ScalarVariableStatistics& other)
{
  ScalarVariableMeanAndVariance::push(other);
  if (other.minimumValue < minimumValue)
    minimumValue = other.minimumValue;
  if (other.maximumValue > maximumValue)
    maximumValue = other.maximumValue;
}

string ScalarVariableStatistics::toString() const
{
  return ScalarVariableMeanAndVariance::toString() + " [" +
    string(minimumValue) + " - " + string(maximumValue) + "]";
}

string ScalarVariableStatistics::toShortString() const
  {return ScalarVariableMeanAndVariance::toShortString();}

int ScalarVariableStatistics::getMinimum(LuaState& state)
{
  ScalarVariableStatisticsPtr stats = state.checkObject(1, scalarVariableStatisticsClass).staticCast<ScalarVariableStatistics>();
  state.pushNumber(stats->getMinimum());
  return 1;
}

int ScalarVariableStatistics::getMaximum(LuaState& state)
{
  ScalarVariableStatisticsPtr stats = state.checkObject(1, scalarVariableStatisticsClass).staticCast<ScalarVariableStatistics>();
  state.pushNumber(stats->getMaximum());
  return 1;
}

/*
** ScalarVariableRecentMean
*/
ScalarVariableRecentMean::ScalarVariableRecentMean(const string& name, size_t memorySize)
  : NameableObject(name), memorySize(memorySize), currentSum(0.0), epoch(0) {}

void ScalarVariableRecentMean::push(double value)
{
  currentSum += value;
  values.push_back(value);
  if (values.size() > memorySize)
  {
    currentSum -= values.front();
    values.pop_front();
  }
  if ((++epoch % 10000) == 0)
    recomputeCurrentSum(); // to avoid numerical errors accumulation
}

void ScalarVariableRecentMean::recomputeCurrentSum()
{
#ifdef JUCE_DEBUG
  double previousSum = currentSum;
#endif // JUCE_DEBUG
  currentSum = 0.0;
  for (std::deque<double>::const_iterator it = values.begin(); it != values.end(); ++it)
    currentSum += *it;
  jassert(fabs(previousSum - currentSum) < 1e-9);
}

/*
** ScalarVariableRecentMeanAndVariance
*/
ScalarVariableRecentMeanAndVariance::ScalarVariableRecentMeanAndVariance(const string& name, size_t memorySize)
  : ScalarVariableRecentMean(name, memorySize), meansqr(name + T(" sqr"), memorySize)
{
}
