/*-----------------------------------------.---------------------------------.
| Filename: DoubleVectorFunctions.h        | DoubleVector Functions          |
| Author  : Francis Maes                   |                                 |
| Started : 23/12/2011 12:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_DOUBLE_VECTOR_H_
# define LBCPP_ML_FUNCTION_DOUBLE_VECTOR_H_

# include "ObjectFunctions.h"
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class GetDoubleVectorElementFunction : public UnaryObjectFunction<GetDoubleVectorElementFunction>
{
public:
  GetDoubleVectorElementFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t index = 0)
    : UnaryObjectFunction<GetDoubleVectorElementFunction>(doubleVectorClass()), enumeration(enumeration), index(index) {}

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

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return inputs[0]->toShortString() + "." + enumeration->getElementName(index);}

  ObjectPtr computeObject(const ObjectPtr& input) const
    {return input.staticCast<DoubleVector>()->getElement(index).toObject();}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
    {return inputs[0] ? computeObject(inputs[0]) : ObjectPtr();}

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
  friend class GetDoubleVectorElementFunctionClass;
  EnumerationPtr enumeration;
  size_t index;

  TypePtr outputType;
};

class ScalarVariableStatisticsPerception : public Object
{
public:
  ScalarVariableStatisticsPerception() : mean(0.0), stddev(0.0), min(0.0), max(0.0), sum(0.0), l0norm(0.0), l1norm(0.0), l2norm(0.0) {}

  double mean;
  double stddev;
  double min;
  double max;
  double sum;
  double l0norm;
  double l1norm;
  double l2norm;
};

typedef ReferenceCountedObjectPtr<ScalarVariableStatisticsPerception> ScalarVariableStatisticsPerceptionPtr;
extern ClassPtr scalarVariableStatisticsPerceptionClass;

class ComputeDoubleVectorStatisticsFunction : public UnaryObjectFunction<ComputeDoubleVectorStatisticsFunction>
{
public:
  ComputeDoubleVectorStatisticsFunction() : UnaryObjectFunction<ComputeDoubleVectorStatisticsFunction>(doubleVectorClass()) {}

  virtual String toShortString() const
    {return "stats(.)";}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return scalarVariableStatisticsPerceptionClass;}

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {jassert(inputs.size() == 1); return "stats(" + inputs[0]->toShortString() + ")";}

  ObjectPtr computeObject(const ObjectPtr& object) const
  {
    const DoubleVectorPtr& vector = object.staticCast<DoubleVector>();
    ScalarVariableStatistics stats;
    size_t l0norm = 0;
    double l1norm = 0.0;
    DenseDoubleVectorPtr denseVector = vector.dynamicCast<DenseDoubleVector>();
    if (denseVector)
    {
      for (size_t i = 0; i < denseVector->getNumValues(); ++i)
      {
        double value = denseVector->getValue(i);
        stats.push(value);
        if (value != 0.0)
        {
          ++l0norm;
          l1norm += fabs(value);
        }
      }
    }
    else
    {
      jassert(false); // not implemented yet
    }
    ScalarVariableStatisticsPerceptionPtr res = new ScalarVariableStatisticsPerception();
    res->mean = stats.getMean();
    res->stddev = stats.getStandardDeviation();
    res->min = stats.getMinimum();
    res->max = stats.getMaximum();
    res->sum = stats.getSum();
    res->l0norm = (double)l0norm;
    res->l1norm = l1norm;
    res->l2norm = sqrt(stats.getSumOfSquares());
    return res;
  }

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
    {return inputs[0] ? computeObject(inputs[0]) : ObjectPtr();}
};

class GetDoubleVectorExtremumsFunction : public UnaryObjectFunction<GetDoubleVectorExtremumsFunction>
{
public:
  GetDoubleVectorExtremumsFunction(EnumerationPtr enumeration = EnumerationPtr())
    : UnaryObjectFunction<GetDoubleVectorExtremumsFunction>(doubleVectorClass()), enumeration(enumeration) {}

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

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return T("extremums(") + inputs[0]->toShortString() + T(")");}

  ObjectPtr computeObject(const ObjectPtr& input) const
  {
    const DoubleVectorPtr& vector = input.dynamicCast<DoubleVector>();
    int argmin = vector->getIndexOfMinimumValue();
    int argmax = vector->getIndexOfMaximumValue();

    return new Pair(outputClass, 
      argmin >= 0 ? Variable(argmin, enumeration) : Variable::missingValue(enumeration),
      argmax >= 0 ? Variable(argmax, enumeration) : Variable::missingValue(enumeration));
  }

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
    {return inputs[0] ? computeObject(inputs[0]) : ObjectPtr();}

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
  friend class GetDoubleVectorExtremumsFunctionClass;

  EnumerationPtr enumeration;
  ClassPtr outputClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_DOUBLE_VECTOR_H_
