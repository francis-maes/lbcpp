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

# include "predeclarations.h"
# include "../Core/Object.h"
# include <cfloat>
# include <deque>

namespace lbcpp
{

class ScalarVariableMean : public NameableObject
{
public:
  ScalarVariableMean(const String& name = String::empty);

  virtual void clear()
    {samplesSum = samplesCount = 0.0;}

  virtual void push(double val)
    {samplesSum += val; samplesCount += 1.0;}

  virtual void push(double val, double weight)
    {samplesSum += weight * val; samplesCount += weight;}

  double getMean() const
    {return samplesCount ? samplesSum / samplesCount : 0.0;}

  double getCount() const
    {return samplesCount;}

  double getSum() const
    {return samplesSum;}

  virtual String toString() const;
  virtual String toShortString() const;

  // Lua
  static int clear(LuaState& state);
  static int observe(LuaState& state);

  static int getMean(LuaState& state);
  static int getSum(LuaState& state);
  static int getCount(LuaState& state);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ScalarVariableMeanClass;

  double samplesSum;
  double samplesCount;
};

typedef ReferenceCountedObjectPtr<ScalarVariableMean> ScalarVariableMeanPtr;
extern ClassPtr scalarVariableMeanClass;

class ScalarVariableMeanAndVariance : public ScalarVariableMean
{
public:
  ScalarVariableMeanAndVariance(const String& name = String::empty);

  void clear()
    {ScalarVariableMean::clear(); samplesSumOfSquares = 0.0;}
  
  void push(double val)
    {ScalarVariableMean::push(val); samplesSumOfSquares += sqr(val);}

  void push(double val, double weight)
    {ScalarVariableMean::push(val, weight); samplesSumOfSquares += sqr(val) * weight;}

  double getSquaresMean() const
    {return samplesSumOfSquares / samplesCount;}

  double getVariance() const // mean(sqr(x)) - sqr(mean(x))
    {return samplesCount ? getSquaresMean() - sqr(getMean()) : 0.0;}

  double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

  virtual String toString() const;
  virtual String toShortString() const;
 
  // Lua
  static int getStandardDeviation(LuaState& state);
  static int getVariance(LuaState& state);

private:
  friend class ScalarVariableMeanAndVarianceClass;

  double samplesSumOfSquares;

  static inline double sqr(double x)
    {return x * x;}
};

typedef ReferenceCountedObjectPtr<ScalarVariableMeanAndVariance> ScalarVariableMeanAndVariancePtr;
extern ClassPtr scalarVariableMeanAndVarianceClass;

class ScalarVariableStatistics : public ScalarVariableMeanAndVariance
{
public:
  ScalarVariableStatistics(const String& name = String::empty);

  void push(double val);
  void push(const std::vector<double>& values);
  void push(double val, double weight);

  double getMinimum() const
    {return minimumValue;}

  double getMaximum() const
    {return maximumValue;}

  double getRange() const
    {return maximumValue - minimumValue;}

  virtual String toString() const;
  virtual String toShortString() const;

  // Lua
  static int getMinimum(LuaState& state);
  static int getMaximum(LuaState& state);

private:
  friend class ScalarVariableStatisticsClass;

  double minimumValue;
  double maximumValue;
};

typedef ReferenceCountedObjectPtr<ScalarVariableStatistics> ScalarVariableStatisticsPtr;
extern ClassPtr scalarVariableStatisticsClass;

class ScalarVariableRecentMean : public NameableObject
{
public:
  ScalarVariableRecentMean(const String& name = T("Unnamed"), size_t memorySize = 0);

  void clear()
    {values.clear(); currentSum = 0.0; epoch = 0;}

  void push(double value);

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

  void recomputeCurrentSum();
};

class ScalarVariableRecentMeanAndVariance : public ScalarVariableRecentMean
{
public:
  ScalarVariableRecentMeanAndVariance(const String& name = T("Unnamed"), size_t memorySize = 0);

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

typedef ReferenceCountedObjectPtr<ScalarVariableRecentMeanAndVariance> ScalarVariableRecentMeanAndVariancePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_VARIABLE_H_
