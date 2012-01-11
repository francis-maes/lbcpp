/*-----------------------------------------.---------------------------------.
| Filename: DoubleVectorLuapeFunctions.h   | DoubleVector Luape Functions    |
| Author  : Francis Maes                   |                                 |
| Started : 23/12/2011 12:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_DOUBLE_VECTOR_H_
# define LBCPP_LUAPE_FUNCTION_DOUBLE_VECTOR_H_

# include "ObjectLuapeFunctions.h"
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class GetDoubleVectorElementLuapeFunction : public UnaryObjectLuapeFuntion<GetDoubleVectorElementLuapeFunction>
{
public:
  GetDoubleVectorElementLuapeFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t index = 0)
    : UnaryObjectLuapeFuntion<GetDoubleVectorElementLuapeFunction>(doubleVectorClass()), enumeration(enumeration), index(index) {}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(type);
    return enumeration ? enumeration == features : features && features->getNumElements() > 0;
  }

  virtual TypePtr initialize(const TypePtr* inputTypes)
  {
    EnumerationPtr features;
    DoubleVector::getTemplateParameters(defaultExecutionContext(), inputTypes[0], features, outputType);
    jassert(features == enumeration);
    return outputType;
  }

  virtual String toShortString() const
    {return T(".") + enumeration->getElementName(index);}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + "." + enumeration->getElementName(index);}

  Variable computeObject(const ObjectPtr& input) const
  {
    DenseDoubleVectorPtr denseInput = input.dynamicCast<DenseDoubleVector>();
    if (denseInput)
      return denseInput->getValue(index);
    else
      return input.staticCast<DoubleVector>()->getElement(index);
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(outputType);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(inputTypes[0]);
    if (!features || features->getNumElements() == 0)
      return ContainerPtr();

    if (index == 0)
    {
      ObjectVectorPtr res = new ObjectVector(enumerationClass, 1);
      res->set(0, features);
      return res;
    }
    else
    {
      size_t n = features->getNumElements();
      VectorPtr res = vector(positiveIntegerType, n);
      for (size_t i = 0; i < n; ++i)
        res->setElement(i, i);
      return res;
    }
  }

protected:
  friend class GetDoubleVectorElementLuapeFunctionClass;
  EnumerationPtr enumeration;
  size_t index;

  TypePtr outputType;
};

class ScalarVariableStatisticsPerception : public Object
{
public:
  ScalarVariableStatisticsPerception() : mean(0.0), stddev(0.0), sum(0.0), min(0.0), max(0.0), percentActive(0.0) {}

  double mean;
  double stddev;
  double sum;
  double min;
  double max;
  double percentActive;
};

typedef ReferenceCountedObjectPtr<ScalarVariableStatisticsPerception> ScalarVariableStatisticsPerceptionPtr;
extern ClassPtr scalarVariableStatisticsPerceptionClass;

class ComputeDoubleVectorStatisticsLuapeFunction : public UnaryObjectLuapeFuntion<ComputeDoubleVectorStatisticsLuapeFunction>
{
public:
  ComputeDoubleVectorStatisticsLuapeFunction() : UnaryObjectLuapeFuntion<ComputeDoubleVectorStatisticsLuapeFunction>(doubleVectorClass()) {}

  virtual String toShortString() const
    {return "stats(.)";}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return scalarVariableStatisticsPerceptionClass;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {jassert(inputs.size() == 1); return "stats(" + inputs[0]->toShortString() + ")";}

  Variable computeObject(const ObjectPtr& object) const
  {
    const DoubleVectorPtr& vector = object.staticCast<DoubleVector>();
    ScalarVariableStatistics stats;
    size_t numActive = 0;
    DenseDoubleVectorPtr denseVector = vector.dynamicCast<DenseDoubleVector>();
    if (denseVector)
    {
      for (size_t i = 0; i < denseVector->getNumValues(); ++i)
      {
        double value = denseVector->getValue(i);
        stats.push(value);
        if (value != 0.0)
          ++numActive;
      }
    }
    else
    {
      jassert(false); // not implemented yet
    }
    ScalarVariableStatisticsPerceptionPtr res = new ScalarVariableStatisticsPerception();
    res->mean = stats.getMean();
    res->stddev = stats.getStandardDeviation();
    res->sum = stats.getSum();
    res->min = stats.getMinimum();
    res->max = stats.getMaximum();
    res->percentActive = (double)numActive / stats.getCount();
    return res;
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(scalarVariableStatisticsPerceptionClass);
  }
};

class GetDoubleVectorExtremumsLuapeFunction : public UnaryObjectLuapeFuntion<GetDoubleVectorExtremumsLuapeFunction>
{
public:
  GetDoubleVectorExtremumsLuapeFunction(EnumerationPtr enumeration = EnumerationPtr())
    : UnaryObjectLuapeFuntion<GetDoubleVectorExtremumsLuapeFunction>(doubleVectorClass()), enumeration(enumeration) {}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(type);
    return enumeration ? enumeration == features : features != EnumerationPtr();
  }

  virtual TypePtr initialize(const TypePtr* inputTypes)
  {
    EnumerationPtr features;
    TypePtr elementsType;
    DoubleVector::getTemplateParameters(defaultExecutionContext(), inputTypes[0], features, elementsType);
    jassert(features == enumeration);
    outputClass = pairClass(features, features);
    return outputClass;
  }

  virtual String toShortString() const
    {return T("extremums(.)");}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return T("extremums(") + inputs[0]->toShortString() + T(")");}

  Variable computeObject(const ObjectPtr& input) const
  {
    const DoubleVectorPtr& vector = input.dynamicCast<DoubleVector>();
    int argmin = vector->getIndexOfMinimumValue();
    int argmax = vector->getIndexOfMaximumValue();

    return new Pair(outputClass, 
      argmin >= 0 ? Variable(argmin, enumeration) : Variable::missingValue(enumeration),
      argmax >= 0 ? Variable(argmax, enumeration) : Variable::missingValue(enumeration));
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(outputClass);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(inputTypes[0]);
    if (!features || features->getNumElements() == 0)
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(enumerationClass, 1);
    res->set(0, features);
    return res;
  }

protected:
  friend class GetDoubleVectorExtremumsLuapeFunctionClass;

  EnumerationPtr enumeration;
  ClassPtr outputClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_DOUBLE_VECTOR_H_
