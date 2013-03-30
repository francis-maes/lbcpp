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

#ifndef ML_RANDOM_VARIABLE_H_
# define ML_RANDOM_VARIABLE_H_

# include "predeclarations.h"
# include <oil/Core/Object.h>
# include <cfloat>
# include <deque>

namespace lbcpp
{

class ScalarVariableMean : public NameableObject
{
public:
  ScalarVariableMean(const string& name = string::empty);

  virtual void clear()
    {samplesSum = samplesCount = 0.0;}

  virtual void push(double val)
    {samplesSum += val; samplesCount += 1.0;}

  virtual void push(double val, double weight)
    {samplesSum += weight * val; samplesCount += weight;}

  virtual double getMean() const
    {return samplesCount ? samplesSum / samplesCount : 0.0;}

  double getCount() const
    {return samplesCount;}

  double getSum() const
    {return samplesSum;}

  virtual string toString() const;
  virtual string toShortString() const;
  
  virtual double toDouble() const
    {return getMean();}

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
  ScalarVariableMeanAndVariance(const string& name = string::empty);

  virtual void clear()
    {ScalarVariableMean::clear(); samplesSumOfSquares = 0.0;}
  
  virtual void push(double val)
    {ScalarVariableMean::push(val); samplesSumOfSquares += sqr(val);}

  virtual void push(double val, double weight)
    {ScalarVariableMean::push(val, weight); samplesSumOfSquares += sqr(val) * weight;}

  virtual double getSquaresMean() const
    {return samplesSumOfSquares / samplesCount;}

  virtual double getSumOfSquares() const
    {return samplesSumOfSquares;}
  
  virtual double getVariance() const // mean(sqr(x)) - sqr(mean(x))
    {return samplesCount ? getSquaresMean() - sqr(getMean()) : 0.0;}

  virtual double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

  virtual string toString() const;
  virtual string toShortString() const;
 
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

/**
 *  Use this class to store a mean and variance that was not calculated from a population (e.g. multiple push() operations)
 */
class ScalarVariableConstMeanAndVariance : public ScalarVariableMeanAndVariance
{
public:
  ScalarVariableConstMeanAndVariance(const string& name = string::empty) 
    : ScalarVariableMeanAndVariance(name), mean(0.0), variance(1.0) {}
  ScalarVariableConstMeanAndVariance(double mean, double variance, const string& name = string::empty) 
    : ScalarVariableMeanAndVariance(name), mean(mean), variance(variance) {}
  
  virtual void clear() 
    {jassertfalse;}
  
  virtual void push(double val)
    {jassertfalse;}
  
  virtual void push(double val, double weight)
    {jassertfalse;}
  
  virtual double getSquaresMean() const
    {jassertfalse; return 0.0;}
  
  virtual double getSumOfSquares() const
    {jassertfalse; return 0.0;}
  
  virtual double getMean() const
    {return mean;}
  
  virtual double getVariance() const
    {return variance;}
  
  virtual double getStandardDeviation() const
    {return variance > DBL_EPSILON ? sqrt(variance) : 0.0;}
  
protected:
  friend class ScalarVariableConstMeanAndVarianceClass;
  
  double mean;
  double variance;
};

typedef ReferenceCountedObjectPtr<ScalarVariableConstMeanAndVariance> ScalarVariableConstMeanAndVariancePtr;
extern ClassPtr scalarVariableConstMeanAndVarianceClass;

class ScalarVariableStatistics : public ScalarVariableMeanAndVariance
{
public:
  ScalarVariableStatistics(const string& name = string::empty);

  virtual void clear();
  virtual void push(double val);
  virtual void push(double val, double weight);

  void push(const std::vector<double>& values);

  double getMinimum() const
    {return minimumValue;}

  double getMaximum() const
    {return maximumValue;}

  double getRange() const
    {return maximumValue - minimumValue;}

  virtual string toString() const;
  virtual string toShortString() const;

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
  ScalarVariableRecentMean(const string& name = T("Unnamed"), size_t memorySize = 0);

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
  ScalarVariableRecentMeanAndVariance(const string& name = T("Unnamed"), size_t memorySize = 0);

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

#endif // !ML_RANDOM_VARIABLE_H_
