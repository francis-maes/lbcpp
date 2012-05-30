/*-----------------------------------------.---------------------------------.
| Filename: EmpiricalContinuousDistribution.h | Empirical Distribution       |
| Author  : Francis Maes                   |                                 |
| Started : 04/05/2012 13:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_EMPIRICAL_CONTINUOUS_DISTRIBUTION_H_
# define LBCPP_DATA_EMPIRICAL_CONTINUOUS_DISTRIBUTION_H_

# include <lbcpp/Core/Variable.h>
# include "../../../src/Luape/Function/ObjectLuapeFunctions.h"
# include <algorithm>

namespace lbcpp
{

class EmpiricalContinuousDistribution : public Object
{
public:
  EmpiricalContinuousDistribution() : isSorted(true), sum(0.0), sumOfSquares(0.0) {}

  void observe(double value)
  {
    if (isSorted)
      isSorted = (values.empty() || value >= values.back());
    values.push_back(value);
    sum += value;
    sumOfSquares += value * value;
  }
  
  size_t getNumSamples() const
    {return values.size();}

  double getMean() const
    {return values.size() ? sum / (double)values.size() : 0.0;}

  double getSquaresMean() const
    {return values.size() ? sumOfSquares / (double)values.size() : 0.0;}
  
  double getVariance() const // mean(sqr(x)) - sqr(mean(x))
    {double m = getMean(); return values.size() ? getSquaresMean() - m * m : 0.0;}

  double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

  double getMinimum() const
    {ensureIsSorted(); return values.size() ? values.front() : DBL_MAX;}

  double getMaximum() const
    {ensureIsSorted(); return values.size() ? values.back() : -DBL_MAX;}

  double getPercentile(double p) const
  {
    if (values.empty())
      return 0.0;
    jassert(p >= 0.0 && p <= 1.0);
    size_t index = (size_t)(p * values.size());
    if (index == values.size())
      index = values.size() - 1;
    return values[index];
  }

  double getMedian() const
    {return getPercentile(0.5);}

private:
  std::vector<double> values;
  double sum;
  double sumOfSquares;
  bool isSorted;

  void ensureIsSorted() const
  {
    if (!isSorted)
    {
      EmpiricalContinuousDistribution* pthis = const_cast<EmpiricalContinuousDistribution* >(this);
      std::sort(pthis->values.begin(), pthis->values.end());
      pthis->isSorted = true;
    }
  }
};

typedef ReferenceCountedObjectPtr<EmpiricalContinuousDistribution> EmpiricalContinuousDistributionPtr;
extern ClassPtr empiricalContinuousDistributionClass;

////////////////////////

class EmpiricalContinuousDistributionLuapeAccessor : public UnaryObjectLuapeFunction<EmpiricalContinuousDistributionLuapeAccessor>
{
public:
  EmpiricalContinuousDistributionLuapeAccessor()
    : UnaryObjectLuapeFunction<EmpiricalContinuousDistributionLuapeAccessor>(empiricalContinuousDistributionClass) {}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return doubleType;}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(".") + toShortString();}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const = 0;

  Variable computeObject(const ObjectPtr& object) const
    {return compute(object.staticCast<EmpiricalContinuousDistribution>());}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(doubleType);
  }
};

class EmpiricalContinuousDistributionNumSamplesFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "numSamples";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return (double)distribution->getNumSamples();}
};

class EmpiricalContinuousDistributionMeanFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "mean";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getMean();}
};

extern ClassPtr empiricalContinuousDistributionMeanFunctionClass;

class EmpiricalContinuousDistributionSquaresMeanFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "sqrMean";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getSquaresMean();}
};

class EmpiricalContinuousDistributionVarianceFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "variance";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getVariance();}
};

class EmpiricalContinuousDistributionStandardDeviationFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "stddev";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getStandardDeviation();}
};

extern ClassPtr empiricalContinuousDistributionStandardDeviationFunctionClass;

class EmpiricalContinuousDistributionMinimumFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "minimum";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getMinimum();}
};

class EmpiricalContinuousDistributionMaximumFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "maximum";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getMaximum();}
};

class EmpiricalContinuousDistributionMedianFunction : public EmpiricalContinuousDistributionLuapeAccessor
{
public:
  virtual String toShortString() const
    {return "median";}

  virtual double compute(const EmpiricalContinuousDistributionPtr& distribution) const
    {return distribution->getMedian();}
};
}; /* namespace lbcpp */

#endif // !LBCPP_DATA_EMPIRICAL_CONTINUOUS_DISTRIBUTION_H_
