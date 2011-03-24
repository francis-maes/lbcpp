/*-----------------------------------------.---------------------------------.
| Filename: AccumulateContainerFunction.h  | Accumulate Container Function   |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATE_H_
# define LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATE_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

extern ClassPtr cumulativeScoreVectorClass(TypePtr scoresEnumeration);

class CumulativeScoreVector : public Container
{
public:
  CumulativeScoreVector(ClassPtr thisClass, EnumerationPtr scores, size_t size)
    : Container(thisClass), elementsType(denseDoubleVectorClass(scores)), numElements(size)
  {
    values = new float*[size];
    numSubElements = scores->getNumElements();
    if (size > 0)
    {
      values[0] = new float[scores->getNumElements()];
      for (size_t i = 0; i < numSubElements; ++i)
        values[0][i] = 0.0;
    }
  }

  CumulativeScoreVector() {}

  ~CumulativeScoreVector()
  {
    for (size_t i = 0; i < numElements; ++i)
      delete [] values[i];
    delete [] values;
  }

  virtual TypePtr getElementsType() const
    {return elementsType;}

  void computeStep(size_t index)
  {
    jassert(index < numElements);
    if (index > 0)
    {
      values[index] = new float[numSubElements];
      for (size_t i = 0; i < numSubElements; ++i)
        values[index][i] = values[index - 1][i];
    }
  }

  void incrementValue(size_t i, size_t j, double value)
  {
    jassert(i < numElements && j < numSubElements);
    values[i][j] += (float)value;
  }
  
  double getValue(size_t i, size_t j)
  {
    jassert(i < numElements && j < numSubElements);
    return (double)values[i][j];
  }

  size_t getNumElements(size_t index) const
    {return numSubElements;}

  /* Container */
  virtual size_t getNumElements() const
    {return numElements;}

  virtual Variable getElement(size_t index) const
  {
    jassert(index < numElements);
    DenseDoubleVectorPtr res = new DenseDoubleVector(elementsType);
    for (size_t i = 0; i < numSubElements; ++i)
      res->setValue(i, values[index][i]);
    return res;
  }

  virtual void setElement(size_t index, const Variable& value)
  {
    jassert(index < numElements);
    DenseDoubleVectorPtr res = value.dynamicCast<DenseDoubleVector>();
    jassert(res);
    for (size_t i = 0; i < numSubElements; ++i)
      values[index][i] = (float)res->getValue(i);
  }

  lbcpp_UseDebuggingNewOperator

private:
  TypePtr elementsType;
  size_t numElements;
  size_t numSubElements;
  float** values;
};

typedef ReferenceCountedObjectPtr<CumulativeScoreVector> CumulativeScoreVectorPtr;

class AccumulateContainerFunctionImpl : public Function
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType) = 0;
  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const = 0;

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(anyType);}

  virtual String getOutputPostFix() const
    {return T("Accumulated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr elementsType = Container::getTemplateParameter(inputVariables[0]->getType());
    scoresEnumeration = getScoresEnumeration(context, elementsType);
    return scoresEnumeration ? (TypePtr)cumulativeScoreVectorClass(scoresEnumeration) : TypePtr();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    CumulativeScoreVectorPtr res(new CumulativeScoreVector(getOutputType(), scoresEnumeration, container->getNumElements()));
    accumulate(container, res);
    return res;
  }

protected:
  EnumerationPtr scoresEnumeration;
};

// enum values + missing
class AccumulateEnumerationContainerFunction : public AccumulateContainerFunctionImpl
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType) 
  {
    const EnumerationPtr& enumeration = elementsType.staticCast<Enumeration>();
    return addMissingToEnumerationEnumeration(enumeration);
  }

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      res->computeStep(i);
      res->incrementValue(i, container->getElement(i).getInteger(), 1.0);
    }
  }
};

// enum values + entropy + missing
class AccumulateEnumerationDistributionContainerFunction : public AccumulateContainerFunctionImpl
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType)
  {
    TypePtr distributionElementsType;
    if (!Distribution::getTemplateParameter(context, elementsType, distributionElementsType))
      return EnumerationPtr();
    inputEnumeration = distributionElementsType.dynamicCast<Enumeration>();
    if (!inputEnumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return EnumerationPtr();
    }
    return addMissingToEnumerationEnumeration(addEntropyToEnumerationEnumeration(inputEnumeration));
  }

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      res->computeStep(i);
      EnumerationDistributionPtr distribution = container->getElement(i).getObjectAndCast<EnumerationDistribution>();
      jassert(distribution);
      size_t enumSize = inputEnumeration->getNumElements();
      for (size_t j = 0; j < enumSize; ++j)
        res->incrementValue(i, j, distribution->computeProbability(Variable(j, inputEnumeration)));
      res->incrementValue(i, enumSize, distribution->computeEntropy());
      res->incrementValue(i, enumSize + 1, distribution->computeProbability(Variable(enumSize, inputEnumeration)));
    }
  }

private:
  EnumerationPtr inputEnumeration;
};

// value sum, missing value count
class AccumulateDoubleContainerFunction : public AccumulateContainerFunctionImpl
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType)
    {return existOrMissingEnumeration;}

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      res->computeStep(i);
      Variable element = container->getElement(i);
      if (element.exists())
        res->incrementValue(i, 0, element.getDouble());
      else
        res->incrementValue(i, 1, 1.0);
    }
  }
};

// features + missing
class AccumulateDoubleVectorContainerFunction : public AccumulateContainerFunctionImpl
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr doubleVectorType)
  {
    EnumerationPtr featuresEnumeration;
    TypePtr featuresType;
    if (!DoubleVector::getTemplateParameters(context, doubleVectorType, featuresEnumeration, featuresType))
      return EnumerationPtr();
    return addMissingToEnumerationEnumeration(featuresEnumeration);
  }

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      DoubleVectorPtr vector = container->getElement(i).getObjectAndCast<DoubleVector>();
      res->computeStep(i);

      if (vector)
      {
        size_t n = vector->getNumElements();
        for (size_t j = 0; j < n; ++j)
          res->incrementValue(i, j, vector->getElement(j).getDouble());
      }
      else
        res->incrementValue(i, res->getNumElements(i) - 1, 1.0); // missing
    }
  }
};

class AccumulateContainerFunction : public ProxyFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(anyType);}

  virtual String getOutputPostFix() const
    {return T("Accumulated");}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    if (inputVariables.size() == 1 && inputVariables[0]->getType()->inheritsFrom(containerClass(anyType)))
    {
      TypePtr elementsType;
      if (Container::getTemplateParameter(defaultExecutionContext(), inputVariables[0]->getType(), elementsType))
      {
        if (elementsType.dynamicCast<Enumeration>())
          return new AccumulateEnumerationContainerFunction();
        else if (elementsType->inheritsFrom(doubleType))
          return new AccumulateDoubleContainerFunction();
        else if (elementsType->inheritsFrom(enumerationDistributionClass(anyType)))
          return new AccumulateEnumerationDistributionContainerFunction();
        else if (elementsType->inheritsFrom(doubleVectorClass()))
          return new AccumulateDoubleVectorContainerFunction();
        else
          return FunctionPtr();
      }
    }
    return FunctionPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATE_H_
