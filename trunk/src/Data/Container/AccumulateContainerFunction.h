/*-----------------------------------------.---------------------------------.
| Filename: AccumulateContainerFunction.h  | Accumulate Container Function   |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATE_H_
# define LBCPP_DATA_CONTAINER_FUNCTION_ACCUMULATE_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

extern ClassPtr cumulativeScoreVectorClass(TypePtr scoresEnumeration);

class CumulativeScoreVector : public ObjectVector
{
public:
  CumulativeScoreVector(ClassPtr thisClass, EnumerationPtr scores, size_t size)
    : ObjectVector(thisClass), elementsType(denseDoubleVectorClass(scores))
  {
    objects->resize(size);
    (*objects)[0] = new DenseDoubleVector(elementsType, scores->getNumElements(), 0.0);
  }

  CumulativeScoreVector() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  const DenseDoubleVectorPtr& getVector(size_t index) const
    {return (*objects)[index].staticCast<DenseDoubleVector>();}

  const DenseDoubleVectorPtr& computeStep(size_t i)
  {
    if (i > 0)
      (*objects)[i] = (*objects)[i - 1]->clone(defaultExecutionContext());
    return getVector(i);
  }

  lbcpp_UseDebuggingNewOperator

private:
  TypePtr elementsType;
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
      const DenseDoubleVectorPtr& scores = res->computeStep(i);
      scores->incrementValue(container->getElement(i).getInteger(), 1.0);
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
      const DenseDoubleVectorPtr& scores = res->computeStep(i);
      
      EnumerationDistributionPtr distribution = container->getElement(i).getObjectAndCast<EnumerationDistribution>();
      jassert(distribution);
      size_t enumSize = inputEnumeration->getNumElements();
      for (size_t j = 0; j < enumSize; ++j)
        scores->incrementValue(j, distribution->computeProbability(Variable(j, inputEnumeration)));
      scores->incrementValue(enumSize, distribution->computeEntropy());
      scores->incrementValue(enumSize + 1, distribution->computeProbability(Variable(enumSize, inputEnumeration)));
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
      const DenseDoubleVectorPtr& scores = res->computeStep(i);
      Variable element = container->getElement(i);
      if (element.exists())
        scores->incrementValue(0, element.getDouble());
      else
        scores->incrementValue(1, 1.0);
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
      DenseDoubleVectorPtr scores = res->computeStep(i);
      if (vector)
        vector->addTo(scores);
      else
        scores->incrementValue(scores->getNumElements() - 1, 1.0); // missing
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
