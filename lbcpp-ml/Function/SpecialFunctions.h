/*-----------------------------------------.---------------------------------.
| Filename: SpecialFunctions.h             | Special Functions               |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_SPECIAL_H_
# define LBCPP_ML_FUNCTION_SPECIAL_H_

# include <lbcpp-ml/Function.h>
# include <lbcpp-ml/Expression.h>
# include "DoubleFunctions.h"
# include <algorithm>

namespace lbcpp
{

class StumpFunction : public Function
{
public:
  StumpFunction(double threshold = 0.0) 
    : threshold(threshold) {}

  virtual string toShortString() const
    {return ">= " + string(threshold);}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const ClassPtr& type) const
    {return type->inheritsFrom(doubleClass) || type->inheritsFrom(integerClass);}
  
  virtual ClassPtr initialize(const ClassPtr* inputTypes)
    {return booleanClass;}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return inputs[0]->toShortString() + " >= " + string(threshold);}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    return new Boolean(Double::get(inputs[0]) >= threshold);
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, ClassPtr outputType) const
  {
    const DataVectorPtr& scalars = inputs[0];
    jassert(scalars->size());
    if (scalars->getElementsType() == doubleClass)
    {
      BVectorPtr res = new BVector(scalars->size());
      unsigned char* dest = res->getDataPointer();
      for (DataVector::const_iterator it = scalars->begin(); it != scalars->end(); ++it)
      {
        double value = it.getRawDouble();
        if (value == doubleMissingValue)
          *dest++ = 2;
        else
          *dest++ = (value >= threshold ? 1 : 0);
      }
      return new DataVector(scalars->getIndices(), res);
    }
    else
      return Function::compute(context, inputs, outputType);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<ClassPtr>& inputTypes) const
  {
    /*
    DenseDoubleVectorPtr res = new DenseDoubleVector(0, 0.0);

    ExpressionCachePtr cache = inputs[0]->getCache();
    jassert(cache->isConvertibleToDouble());
    
    const std::vector< std::pair<size_t, double> >& sortedDoubleValues = cache->getSortedDoubleValues();
    if (sortedDoubleValues.size())
    {
      jassert(sortedDoubleValues.size());
      double previousThreshold = sortedDoubleValues[0].second;
      for (size_t i = 0; i < sortedDoubleValues.size(); ++i)
      {
        double threshold = sortedDoubleValues[i].second;
        jassert(threshold >= previousThreshold);
        if (threshold > previousThreshold)
        {
          res->appendValue((threshold + previousThreshold) / 2.0);
          previousThreshold = threshold;
        }
      }
    }
    else
      jassert(false); // no training data, cannot choose thresholds

    return res;*/
    return ContainerPtr();
  }

  double getThreshold() const
    {return threshold;}

protected:
  friend class StumpFunctionClass;

  double threshold;
};

typedef ReferenceCountedObjectPtr<StumpFunction> StumpFunctionPtr;

class GreaterThanDoubleFunction : public Function
{
public:
  virtual string toShortString() const
    {return ">";}

  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const ClassPtr& type) const
    {return type->inheritsFrom(doubleClass);}

  virtual ClassPtr initialize(const ClassPtr* inputTypes)
    {return booleanClass;}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return inputs[0]->toShortString() + T(" > ") + inputs[1]->toShortString();}
  
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0] || !inputs[1])
      return ObjectPtr();
    return new Boolean(Double::get(inputs[0]) > Double::get(inputs[1]));
  }

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class NormalizerFunction : public UnaryDoubleFunction
{
public:
  virtual ClassPtr initialize(const ClassPtr* inputTypes)
    {return probabilityClass;}

  void initialize(const DenseDoubleVectorPtr& inputValues, size_t numPercentiles = 10)
    {computePercentiles(inputValues, numPercentiles, percentiles);}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "normalize(" + inputs[0]->toShortString() + ")";}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    return new Probability(computeDouble(Double::get(inputs[0])));
  }

  virtual double computeDouble(double value) const
  {
    jassert(percentiles.size());
    if (value <= percentiles[0])
      return 0.0;
    for (size_t i = 1; i < percentiles.size(); ++i)
      if (value < percentiles[i])
      {
        double k = (value - percentiles[i - 1]) / (percentiles[i] - percentiles[i - 1]);
        return ((double)i - 1 + k) / (double)(percentiles.size() - 1.0);
      }
    return 1.0;
  }

  lbcpp_UseDebuggingNewOperator

private:
  std::vector<double> percentiles;

  struct CompareValues
  {
    bool operator ()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
      {return a.second != b.second ? a.second < b.second : a.first < b.first;}
  };

  void computePercentiles(const DenseDoubleVectorPtr& values, size_t numPercentiles, std::vector<double>& res)
  {
    size_t n = values->getNumValues();
    std::vector< std::pair<size_t, double> > sortedValues(n);
    for (size_t i = 0; i < n; ++i)
      sortedValues[i] = std::make_pair(i, values->getValue(i));
    std::sort(sortedValues.begin(), sortedValues.end(), CompareValues());

    if (numPercentiles > n)
      numPercentiles = n;
    
    res.resize(numPercentiles);

    double samplesPerPercentile = numPercentiles > 1 ? (n - 1.0) / (numPercentiles - 1.0) : 0.0;
    for (size_t i = 0; i < numPercentiles; ++i)
    {
      size_t index = (size_t)(samplesPerPercentile * i);
      jassert(index < sortedValues.size());
      res[i] = sortedValues[index].second;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_SPECIAL_H_
