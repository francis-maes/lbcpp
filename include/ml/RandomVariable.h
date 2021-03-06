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
# include <Array/Array2D.h>
# include <LinAlg/LinAlg.h>
# include <oil/Core/Object.h>
# include <cfloat>
# include <deque>
# include <ml/DoubleVector.h>

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

  virtual void push(const ScalarVariableMean& other)
    {samplesSum += other.samplesSum; samplesCount += other.samplesCount;}

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

  virtual void push(const ScalarVariableMean& other)
    {jassertfalse;}

  virtual void push(const ScalarVariableMeanAndVariance& other)
    {ScalarVariableMean::push(other); samplesSumOfSquares += other.samplesSumOfSquares;}

  virtual void subtract(const ScalarVariableMeanAndVariance& other)
  {
    samplesSum -= other.samplesSum;
    samplesCount -= other.samplesCount;
    samplesSumOfSquares -= other.samplesSumOfSquares;
  }

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

  virtual void push(const ScalarVariableMean& other)
    {jassertfalse;}

  virtual void push(const ScalarVariableMeanAndVariance& other)
    {jassertfalse;}

  virtual void push(const ScalarVariableConstMeanAndVariance& other)
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

  virtual void push(const ScalarVariableMean& other)
    {jassertfalse;}

  virtual void push(const ScalarVariableMeanAndVariance& other)
    {jassertfalse;}

  virtual void push(const ScalarVariableStatistics& other);

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


class CorrelationCoefficient : public Object
{
public:
  CorrelationCoefficient() {}
  virtual void push(double x, double y) = 0;
  virtual double getCorrelationCoefficient() const = 0;
protected:
};

typedef ReferenceCountedObjectPtr<CorrelationCoefficient> CorrelationCoefficientPtr;

class PearsonCorrelationCoefficient : public CorrelationCoefficient
{
public:
  PearsonCorrelationCoefficient() : numSamples(0), sumXY(0.0), sumX(0.0), sumXsquared(0.0), sumY(0.0), sumYsquared(0.0) {}

  virtual void push(double x, double y)
  {
    ++numSamples;
    sumXY += x * y;
    sumX += x;
    sumXsquared += x * x;
    sumY += y;
    sumYsquared += y * y;
  }

  virtual void push(const PearsonCorrelationCoefficient& other)
  {
    if (other.numSamples == 0)
      return;
    numSamples += other.numSamples;
    sumXY += other.sumXY;
    sumX += other.sumX;
    sumXsquared += other.sumXsquared;
    sumY += other.sumY;
    sumYsquared += other.sumYsquared;
  }

  virtual void subtract(const PearsonCorrelationCoefficient& other)
  {
    if (other.numSamples > numSamples)
      jassertfalse;
    if (other.numSamples == 0)
      return;
    numSamples -= other.numSamples;
    sumXY -= other.sumXY;
    sumX -= other.sumX;
    if (other.sumXsquared > sumXsquared)
      jassertfalse;
    sumXsquared -= other.sumXsquared;
    sumY -= other.sumY;
    if (other.sumYsquared > sumYsquared)
      jassertfalse;
    sumYsquared -= other.sumYsquared;
  }

  virtual double getCorrelationCoefficient() const
    {return (numSamples * sumXY - sumX * sumY) / (sqrt((numSamples - 1) * sumXsquared - sumX * sumX) * sqrt((numSamples - 1) * sumYsquared - sumY * sumY));}

  virtual double getCoefficientOfDetermination() const
  {
    double ss_res = 0;
    double ss_tot = 0;
    double div = numSamples*sumXsquared-sumX*sumX;
    if(div == 0) return 0;
	  double b = (numSamples*sumXY-sumX*sumY)/div;
    if(numSamples == 0) return 0;
	  double a = (sumY-b*sumX)/numSamples;
	  ss_res = sumYsquared-2*a*sumY-2*b*sumXY+numSamples*a*a+2*a*b*sumX+b*b*sumXsquared;
    ss_tot = sumYsquared - sumY*sumY/numSamples;
    if(ss_tot == 0) return 0;
    return 1 - ss_res/ss_tot;
  }

  double getResidualStandardDeviation() const
  {
    if (numSamples < 2)
      return DBL_MAX;
    // residual variance
    double tmp = sumXY - sumX * sumY / numSamples;
    double rv = (sumYsquared - sumY * sumY / numSamples - tmp * tmp / (sumXsquared - sumX * sumX / numSamples)) / (numSamples - 1);
    // check small numerical errors
    if (rv < 0.0 && rv > -1.0e-3)
      return 0.0;
    else if (rv < 0)
      jassertfalse;
    double rsd = sqrt(rv);
    return rsd;
  }

  /** Calculate the slope of the simple linear regressor fitted to the data pushed into this object
   */
  double getSlope() const
    {return (numSamples * sumXY - sumX * sumY) / (numSamples * sumXsquared - sumX * sumX);}

  double getIntercept() const
    {return (sumY - getSlope() * sumX) / numSamples;}

  double getXMean() const
    {return sumX / numSamples;}

  double getYMean() const
    {return sumY / numSamples;}

  size_t getNumSamples() const
    {return numSamples;}

  size_t numSamples;
  double sumXY;
  double sumX, sumXsquared;
  double sumY, sumYsquared;

protected:
  friend class PearsonCorrelationCoefficientClass;
};

typedef ReferenceCountedObjectPtr<ScalarVariableMean> ScalarVariableMeanPtr;
typedef ReferenceCountedObjectPtr<PearsonCorrelationCoefficient> PearsonCorrelationCoefficientPtr;

class MultiVariateRegressionStatistics : public Object
{
public:
  MultiVariateRegressionStatistics() : xtx(Array<double>(1,1)), xty(Array<double>(1,1)), cachedWeightsValid(false) {}

  MultiVariateRegressionStatistics(const MultiVariateRegressionStatistics& other) : cachedWeightsValid(false)
  {
    stats = std::vector<PearsonCorrelationCoefficientPtr>(other.getNumAttributes());
    for (size_t i = 0; i < stats.size(); ++i)
      stats[i] = new PearsonCorrelationCoefficient(*other.getStats(i));
    sumXiXj = std::vector<double>(other.sumXiXj.begin(), other.sumXiXj.end());
    xtx = Array<double>(other.xtx);
    xty = Array<double>(other.xty);
  }

  virtual void push(const DenseDoubleVectorPtr& attributes, double y)
  {
    cachedWeightsValid = false;
    if (stats.size() == 0)
      for (size_t i = 0; i < attributes->getNumValues(); ++i)
        stats.push_back(new PearsonCorrelationCoefficient());
    jassert(attributes->getNumValues() == stats.size());
    for (size_t i = 0; i < attributes->getNumValues(); ++i)
      stats[i]->push(attributes->getValue(i), y);

    size_t numAttr = attributes->getNumValues() + 1;
    DenseDoubleVectorPtr extendedInput = new DenseDoubleVector(numAttr, 1.0);
    for (size_t i = 1; i < numAttr; ++i)
      extendedInput->setValue(i, attributes->getValue(i - 1));
    
        // initialise sumXiXj vector
    if (sumXiXj.size() == 0)
      for (size_t i = 1; i < attributes->getNumValues(); ++i)
        for (size_t j = 0; j < i; ++j)
          sumXiXj.push_back(0.0);
    size_t count = 0;
    for (size_t i = 1; i < attributes->getNumValues(); ++i)
      for (size_t j = 0; j < i; ++j)
        sumXiXj[count++] += attributes->getValue(i) * attributes->getValue(j);

    // initialise xtx and xty matrices
    if (xtx.dim(0) != numAttr)
    {
      xtx.resize(numAttr, numAttr, false);
      xty.resize(numAttr, 1, false);
      for (size_t i = 0; i < numAttr; ++i)
      {
        for (size_t j = 0; j < numAttr; ++j)
          xtx(i,j) = 0.0;
        xty(i,0) = 0.0;
      }
    }
    
    for (size_t i = 0; i < numAttr; ++i)
    {
      xtx(i, i) += extendedInput->getValue(i) * extendedInput->getValue(i);
      for (size_t j = i + 1; j < numAttr; ++j)
      {
        double val = extendedInput->getValue(i) * extendedInput->getValue(j);
        xtx(i, j) += val;
        xtx(j, i) += val;
      }
      xty(i, 0) += extendedInput->getValue(i) * y;
    }
  }

  virtual void update(const MultiVariateRegressionStatistics& other)
  {
    cachedWeightsValid = false;
    if (other.stats.size() == 0)
      return;
    // initialise pearson regression statistics
    if (stats.size() == 0)
      for (size_t i = 0; i < other.getNumAttributes(); ++i)
        stats.push_back(new PearsonCorrelationCoefficient());
    for (size_t i = 0; i < other.getNumAttributes(); ++i)
      stats[i]->push(*other.getStats(i));

    // initialise sumXiXj vector
    if (sumXiXj.size() == 0)
      sumXiXj = std::vector<double>(other.sumXiXj.begin(), other.sumXiXj.end());
    else
      for (size_t i = 0; i < other.sumXiXj.size(); ++i)
        sumXiXj[i] += other.sumXiXj[i];
    
    // initialise xtx and xty matrices
    size_t numAttr = other.xtx.dim(0);
    if (xtx.dim(0) != numAttr)
    {
      xtx.resize(numAttr, numAttr, false);
      xty.resize(numAttr, 1, false);
      for (size_t i = 0; i < numAttr; ++i)
      {
        for (size_t j = 0; j < numAttr; ++j)
          xtx(i,j) = 0.0;
        xty(i,0) = 0.0;
      }
    }

    for (size_t i = 0; i < numAttr; ++i)
    {
      for (size_t j = 0; j < numAttr; ++j)
        xtx(i, j) += other.xtx(i,j);
      xty(i, 0) += other.xty(i, 0);
    }
  }

  /**
   * Subtract the statistics of an other MultiVariateRegressionStatistics from this one
   */
  virtual void subtract(const MultiVariateRegressionStatistics& other)
  {
    cachedWeightsValid = false;
    if (other.stats.size() == 0)
      return;
    for (size_t i = 0; i < other.getNumAttributes(); ++i)
      stats[i]->subtract(*other.getStats(i));

    for (size_t i = 0; i < other.sumXiXj.size(); ++i)
      sumXiXj[i] -= other.sumXiXj[i];
    
    size_t numAttr = other.xtx.dim(0);
    for (size_t i = 0; i < numAttr; ++i)
    {
      for (size_t j = 0; j < numAttr; ++j)
        xtx(i, j) -= other.xtx(i,j);
      xty(i, 0) -= other.xty(i, 0);
    }
  }

  // assumes all variables are uncorrelated
  virtual double getCoefficientOfDetermination() const
  {
    double correlation = 0;
    for (size_t i = 0; i < getNumAttributes(); ++i)
      correlation += getStats(i)->getCoefficientOfDetermination();
    return correlation;
  }

  size_t getNumAttributes() const
    {return stats.size();}

  size_t getExamplesSeen() const
  {
    if (stats.size() == 0) 
      return 0;
    return stats[0]->getNumSamples();
  }

  /** Calculate the linear least squares parameter estimation
   *
   */
  DenseDoubleVectorPtr getLLSQEstimate() const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(xtx.dim(0), 0.0);
    if (cachedWeightsValid)
    {
      for (size_t i = 0; i < cachedWeights->getNumValues(); ++i)
        result->setValue(i, cachedWeights->getValue(i));
    }
    else
    {
      Array<double> xtxinv(xtx.dim(0), xtx.dim(1));
      xtxinv = invert(xtx);
      Array<double> b;
      matMat(b, xtxinv, xty);

      const_cast<MultiVariateRegressionStatistics*>(this)->cachedWeights = new DenseDoubleVector(xtx.dim(0), 0.0);

      for (size_t i = 0; i < b.dim(0); ++i)
      {
        const_cast<MultiVariateRegressionStatistics*>(this)->cachedWeights->setValue(i, b(i));
        result->setValue(i, b(i));
      }
    }
    const_cast<MultiVariateRegressionStatistics*>(this)->cachedWeightsValid = true;
    return result;
  }

  /**
   * Calculate the slope of the simple linear regressor fitted to attribute i
   */
  double getSlope(size_t i) const
  {
    if (getExamplesSeen() < 2)
      return 0.0;
    return stats[i]->getSlope();
  }

  /**
   * Calculate the intercept of the simple linear regressor fitted to attribute i
   */
  double getIntercept(size_t i) const
  {
    if (getExamplesSeen() == 0)
      return 0.0;
    return stats[i]->getIntercept();
  }

  /**
   * Calculate the residual standard deviation of the simple linear regressor fitted to attribute i
   */
  double getResidualStandardDeviation(size_t i) const
  {
    if (getExamplesSeen() < 2)
      return DBL_MAX;
    double b = stats[i]->getSlope();
    double rsd = sqrt((stats[i]->sumYsquared - stats[i]->sumY * stats[i]->sumY / stats[i]->numSamples - 2 * b * (stats[i]->sumXY - stats[i]->sumX * stats[i]->sumY / stats[i]->numSamples) + b * b * (stats[i]->sumXsquared - stats[i]->sumX * stats[i]->sumX / stats[i]->numSamples)) / (stats[i]->numSamples - 1));
    return rsd;
  }

  /**
   * Calculate the residual standard deviation of the complete linear model
   */
  double getResidualStandardDeviation() const
  {
    if (getExamplesSeen() < 2 * stats.size())
      return DBL_MAX;
    DenseDoubleVectorPtr weights = getLLSQEstimate();

    double term1 = stats[0]->sumYsquared;
    double term2 = 0.0;
    for (size_t i = 1; i < weights->getNumValues(); ++i)
    {
      term1 -= 2 * weights->getValue(i) * stats[i-1]->sumXY;
      term1 += weights->getValue(i) * weights->getValue(i) * stats[i-1]->sumXsquared;
      term2 += weights->getValue(i) * stats[i-1]->sumX;
    }
    term2 = -2 * stats[0]->sumY * term2 + stats[0]->sumY * stats[0]->sumY;
    double tmp = 0.0;
    for (size_t i = 1; i < weights->getNumValues(); ++i)
      tmp += weights->getValue(i) * stats[i-1]->sumX;
    term2 += tmp * tmp;
    size_t count = 0;
    for (size_t i = 1; i < getNumAttributes(); ++i)
      for (size_t j = 0; j < i; ++j)
        term1 += 2 * weights->getValue(i + 1) * weights->getValue(j + 1) * sumXiXj[count++];


    double rv = (term1 - term2 / stats[0]->numSamples) / (stats[0]->numSamples - 1);
    if (rv < 0.0 && rv > -1.0e-3)
      return 0.0;
    else if (rv < 0.0)
      jassertfalse;
    double rsd = sqrt(rv);
    return rsd;
  }

  Array<double> getXTX() const
    {return xtx;}

  Array<double> getXTY() const
    {return xty;}

  virtual PearsonCorrelationCoefficientPtr getStats(size_t idx) const
  {
    if (stats.size() == 0) return new PearsonCorrelationCoefficient();
    return stats[idx];
  }

  virtual ObjectPtr clone(ExecutionContext& context)
  {
    ReferenceCountedObjectPtr<MultiVariateRegressionStatistics> result = new MultiVariateRegressionStatistics();
    result->stats = std::vector<PearsonCorrelationCoefficientPtr>(stats.size());
    for (size_t i = 0; i < stats.size(); ++i)
      result->stats[i] = new PearsonCorrelationCoefficient(*stats[i]);
    result->xtx = Array<double>(xtx);
    result->xty = Array<double>(xty);
    return result;
  }

protected:
  friend class MultiVariateRegressionStatisticsClass;

  std::vector<PearsonCorrelationCoefficientPtr> stats;
  std::vector<double> sumXiXj;
  Array<double> xtx;
  Array<double> xty;

  DenseDoubleVectorPtr cachedWeights;
  bool cachedWeightsValid;
};

typedef ReferenceCountedObjectPtr<MultiVariateRegressionStatistics> MultiVariateRegressionStatisticsPtr;

}; /* namespace lbcpp */

#endif // !ML_RANDOM_VARIABLE_H_
