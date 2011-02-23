/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: RandomVariable.h               | Random variable statistics      |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_RANDOM_VARIABLE_H_
# define LBCPP_RANDOM_VARIABLE_H_

# include "../Data/predeclarations.h"
# include <cfloat>
# include <deque>

namespace lbcpp
{

class ScalarVariableMean : public NameableObject
{
public:
  ScalarVariableMean(const String& name = String::empty)
    : NameableObject(name), samplesSum(0.0), samplesCount(0.0) {}

  void clear()
    {samplesSum = samplesCount = 0.0;}

  void push(double val)
    {samplesSum += val; samplesCount += 1.0;}

  void push(double val, double weight)
    {samplesSum += weight * val; samplesCount += weight;}

  double getMean() const
    {return samplesCount ? samplesSum / samplesCount : 0.0;}

  double getCount() const
    {return samplesCount;}

  double getSum() const
    {return samplesSum;}

  virtual String toString() const
  {
    String res;
    String name = getName();
    if (name.isNotEmpty())
      res = name + T(" = ");
    res += String(getMean());
    return res;
  }

  virtual String toShortString() const
    {return String(getMean());}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ScalarVariableMeanClass;

  double samplesSum;
  double samplesCount;
};

typedef ReferenceCountedObjectPtr<ScalarVariableMean> ScalarVariableMeanPtr;

class ScalarVariableMeanAndVariance : public ScalarVariableMean
{
public:
  ScalarVariableMeanAndVariance(const String& name = String::empty)
    : ScalarVariableMean(name), samplesSumOfSquares(0.0) {}

  void clear()
    {ScalarVariableMean::clear(); samplesSumOfSquares = 0.0;}
  
  void push(double val)
    {ScalarVariableMean::push(val); samplesSumOfSquares += sqr(val);}

  void push(double val, double weight)
    {ScalarVariableMean::push(val, weight); samplesSumOfSquares += sqr(val) * weight;}

  double getVariance() const // mean(sqr(x)) - sqr(mean(x))
    {return samplesCount ? samplesSumOfSquares / samplesCount - sqr(getMean()) : 0.0;}

  double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

  virtual String toString() const
    {return ScalarVariableMean::toString() + " +/- " + String(getStandardDeviation());}

  virtual String toShortString() const
  {
    String res = ScalarVariableMean::toShortString();
    double stddev = getStandardDeviation();
    if (stddev)
      res += T(" +/- ") + String(stddev);
    return res;
  }
 
private:
  friend class ScalarVariableMeanAndVarianceClass;

  double samplesSumOfSquares;

  static inline double sqr(double x)
    {return x * x;}
};

typedef ReferenceCountedObjectPtr<ScalarVariableMeanAndVariance> ScalarVariableMeanAndVariancePtr;

class ScalarVariableStatistics : public ScalarVariableMeanAndVariance
{
public:
  ScalarVariableStatistics(const String& name = String::empty)
    : ScalarVariableMeanAndVariance(name), minimumValue(DBL_MAX), maximumValue(-DBL_MAX) {}

  void push(double val)
  {
    ScalarVariableMeanAndVariance::push(val);
    if (val < minimumValue)
	    minimumValue = val;
    if (val > maximumValue)
	    maximumValue = val;
  }

  void push(const std::vector<double>& values)
  {
    for (size_t i = 0; i < values.size(); ++i)
      push(values[i]);
  }

  void push(double val, double weight)
  {
    ScalarVariableMeanAndVariance::push(val, weight);
    if (val < minimumValue)
	    minimumValue = val;
    if (val > maximumValue)
	    maximumValue = val;
  }

  double getMinimum() const
    {return minimumValue;}

  double getMaximum() const
    {return maximumValue;}

  double getRange() const
    {return maximumValue - minimumValue;}

  virtual String toString() const
  {
    return ScalarVariableMeanAndVariance::toString() + " [" +
      String(minimumValue) + " - " + String(maximumValue) + "]";
  }

  virtual String toShortString() const
    {return ScalarVariableMeanAndVariance::toShortString();}

private:
  friend class ScalarVariableStatisticsClass;

  double minimumValue;
  double maximumValue;
};

typedef ReferenceCountedObjectPtr<ScalarVariableStatistics> ScalarVariableStatisticsPtr;

class ScalarVariableRecentMean : public NameableObject
{
public:
  ScalarVariableRecentMean(const String& name = T("Unnamed"), size_t memorySize = 0)
    : NameableObject(name), memorySize(memorySize), currentSum(0.0), epoch(0) {}

  void clear()
    {values.clear(); currentSum = 0.0; epoch = 0;}

  void push(double value)
  {
    currentSum += value;
    values.push_back(value);
    if (values.size() > memorySize)
    {
      currentSum -= values.front();
      values.pop_front();
    }
    if (++epoch % 1000)
      recomputeCurrentSum(); // to avoid numerical errors accumulation
  }

  double getMean() const
    {return values.size() ? currentSum / (double)values.size() : 0.0;}

  size_t getNumSamples() const
    {return values.size();}

  size_t getMemorySize() const
    {return memorySize;}

  bool isMemoryFull() const
    {return values.size() == memorySize;}

  lbcpp_UseDebuggingNewOperator

protected:
  size_t memorySize;
  std::deque<double> values;
  double currentSum;
  size_t epoch;

  void recomputeCurrentSum()
  {
    currentSum = 0.0;
    for (std::deque<double>::const_iterator it = values.begin(); it != values.end(); ++it)
      currentSum += *it;
  }
};

class ScalarVariableRecentMeanAndVariance : public ScalarVariableRecentMean
{
public:
  ScalarVariableRecentMeanAndVariance(const String& name = T("Unnamed"), size_t memorySize = 0)
    : ScalarVariableRecentMean(name, memorySize), meansqr(name + T(" sqr"), memorySize) {}

  void clear()
    {ScalarVariableRecentMean::clear(); meansqr.clear();}

  void push(double value)
    {ScalarVariableRecentMean::push(value); meansqr.push(value * value);}

  double getVariance() const
    {double mean = getMean(); return meansqr.getMean() - mean * mean;}

  double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

private:
  ScalarVariableRecentMean meansqr;
};

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_VARIABLE_H_
