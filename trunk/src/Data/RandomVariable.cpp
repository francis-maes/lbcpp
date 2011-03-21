/*-----------------------------------------.---------------------------------.
| Filename: RandomVariable.cpp             | Random variable statistics      |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/RandomVariable.h>
using namespace lbcpp;

/*
** ScalarVariableMean
*/
ScalarVariableMean::ScalarVariableMean(const String& name)
  : NameableObject(name), samplesSum(0.0), samplesCount(0.0)
{
}

String ScalarVariableMean::toString() const
{
  String res;
  String name = getName();
  if (name.isNotEmpty())
    res = name + T(" = ");
  res += String(getMean());
  return res;
}

String ScalarVariableMean::toShortString() const
  {return String(getMean());}

/*
** ScalarVariableMeanAndVariance
*/
ScalarVariableMeanAndVariance::ScalarVariableMeanAndVariance(const String& name)
  : ScalarVariableMean(name), samplesSumOfSquares(0.0) {}

String ScalarVariableMeanAndVariance::toString() const
  {return ScalarVariableMean::toString() + " +/- " + String(getStandardDeviation());}

String ScalarVariableMeanAndVariance::toShortString() const
{
  String res = ScalarVariableMean::toShortString();
  double stddev = getStandardDeviation();
  if (stddev)
    res += T(" +/- ") + String(stddev);
  return res;
}

/*
** ScalarVariableStatistics
*/
ScalarVariableStatistics::ScalarVariableStatistics(const String& name)
  : ScalarVariableMeanAndVariance(name), minimumValue(DBL_MAX), maximumValue(-DBL_MAX)
{
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

String ScalarVariableStatistics::toString() const
{
  return ScalarVariableMeanAndVariance::toString() + " [" +
    String(minimumValue) + " - " + String(maximumValue) + "]";
}

String ScalarVariableStatistics::toShortString() const
  {return ScalarVariableMeanAndVariance::toShortString();}

/*
** ScalarVariableRecentMean
*/
ScalarVariableRecentMean::ScalarVariableRecentMean(const String& name, size_t memorySize)
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
  double previousSum = currentSum;
  currentSum = 0.0;
  for (std::deque<double>::const_iterator it = values.begin(); it != values.end(); ++it)
    currentSum += *it;
  jassert(fabs(previousSum - currentSum) < 1e-9);
}

/*
** ScalarVariableRecentMeanAndVariance
*/
ScalarVariableRecentMeanAndVariance::ScalarVariableRecentMeanAndVariance(const String& name, size_t memorySize)
  : ScalarVariableRecentMean(name, memorySize), meansqr(name + T(" sqr"), memorySize)
{
}
