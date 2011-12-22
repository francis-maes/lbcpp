/*-----------------------------------------.---------------------------------.
| Filename: SpecialLuapeFunctions.h        | Special Luape Functions         |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_SPECIAL_H_
# define LBCPP_LUAPE_FUNCTION_SPECIAL_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector
# include "DoubleLuapeFunctions.h"
# include <algorithm>

namespace lbcpp
{

class StumpLuapeFunction : public LuapeFunction
{
public:
  StumpLuapeFunction(double threshold = 0.0) 
    : threshold(threshold) {}

  virtual String toShortString() const
    {return "> " + String(threshold);}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(doubleType) || type->inheritsFrom(integerType);}
  
  virtual TypePtr initialize(const std::vector<TypePtr>& )
    {return booleanType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + " >= " + String(threshold);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    double v = inputs[0].toDouble();
    return v == doubleMissingValue ? Variable::missingValue(booleanType) : Variable(v >= threshold, booleanType);
  }

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    const LuapeSampleVectorPtr& scalars = inputs[0];
    jassert(scalars->size());
    if (scalars->getElementsType() == doubleType)
    {
      BooleanVectorPtr res = new BooleanVector(scalars->size());
      unsigned char* dest = res->getData();
      for (LuapeSampleVector::const_iterator it = inputs[0]->begin(); it != inputs[0]->end(); ++it)
      {
        double value = it.getRawDouble();
        if (value == doubleMissingValue)
          *dest++ = 2;
        else
          *dest++ = (value >= threshold ? 1 : 0);
      }
      return new LuapeSampleVector(scalars->getIndices(), res);
    }
    else
      return LuapeFunction::compute(context, inputs, outputType);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    /*
    DenseDoubleVectorPtr res = new DenseDoubleVector(0, 0.0);

    LuapeNodeCachePtr cache = inputs[0]->getCache();
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
  friend class StumpLuapeFunctionClass;

  double threshold;
};

typedef ReferenceCountedObjectPtr<StumpLuapeFunction> StumpLuapeFunctionPtr;

class GreaterThanDoubleLuapeFunction : public LuapeFunction
{
public:
  virtual String toShortString() const
    {return ">";}

  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(doubleType);}

  virtual TypePtr initialize(const std::vector<TypePtr>& )
    {return booleanType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(" > ") + inputs[1]->toShortString();}
  
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    double v1 = inputs[0].getDouble();
    double v2 = inputs[1].getDouble();
    return v1 == doubleMissingValue || v2 == doubleMissingValue ? Variable::missingValue(booleanType) :  Variable(inputs[0].getDouble() > inputs[1].getDouble(), booleanType);
  }

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class NormalizerLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  NormalizerLuapeFunction()
    {vectorClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, probabilityType);}

  virtual TypePtr initialize(const std::vector<TypePtr>& inputTypes)
    {return probabilityType;}

  void initialize(const DenseDoubleVectorPtr& inputValues, size_t numPercentiles = 10)
    {computePercentiles(inputValues, numPercentiles, percentiles);}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "normalize(" + inputs[0]->toShortString() + ")";}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    double value = inputs[0].getDouble();
    return Variable(value == doubleMissingValue ? value : computeDouble(value), probabilityType);
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

#endif // !LBCPP_LUAPE_FUNCTION_SPECIAL_H_
