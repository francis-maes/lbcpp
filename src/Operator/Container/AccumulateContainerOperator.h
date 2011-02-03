/*-----------------------------------------.---------------------------------.
| Filename: AccumulateContainerOperator.h  | Accumulate Container Function   |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_
# define LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_

# include <lbcpp/Operator/Operator.h>
# include <lbcpp/Core/DynamicObject.h>
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
    objects.resize(size);
    objects[0] = new DenseDoubleVector(elementsType, scores->getNumElements(), 0.0);
  }

  CumulativeScoreVector() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  const DenseDoubleVectorPtr& getVector(size_t index) const
    {return objects[index].staticCast<DenseDoubleVector>();}

  const DenseDoubleVectorPtr& computeStep(size_t i)
  {
    if (i > 0)
      objects[i] = objects[i - 1]->clone(defaultExecutionContext());
    return getVector(i);
  }

  lbcpp_UseDebuggingNewOperator

private:
  TypePtr elementsType;
};

typedef ReferenceCountedObjectPtr<CumulativeScoreVector> CumulativeScoreVectorPtr;

class AccumulateContainerOperator : public Function
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType) = 0;
  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const = 0;

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    TypePtr elementsType;
    if (!getContainerElementsType(context, inputVariable->getType(), elementsType))
      return VariableSignaturePtr();
    scoresEnumeration = getScoresEnumeration(context, elementsType);
    if (!scoresEnumeration)
      return VariableSignaturePtr();
    return new VariableSignature(cumulativeScoreVectorClass(scoresEnumeration), inputVariable->getName() + T("Accumulated"), inputVariable->getShortName() + T("a"));
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
class AccumulateEnumerationContainerOperator : public AccumulateContainerOperator
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

// enum values + missing + entropy
class AccumulateEnumerationDistributionContainerOperator : public AccumulateContainerOperator
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType)
  {
    TypePtr distributionElementsType;
    if (!getDistributionElementsType(context, elementsType, distributionElementsType))
      return EnumerationPtr();
    inputEnumeration = distributionElementsType.dynamicCast<Enumeration>();
    if (!inputEnumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return EnumerationPtr();
    }
    return addEntropyToEnumerationEnumeration(addMissingToEnumerationEnumeration(inputEnumeration));
  }

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const DenseDoubleVectorPtr& scores = res->computeStep(i);
      
      EnumerationDistributionPtr distribution = container->getElement(i).getObjectAndCast<EnumerationDistribution>();
      jassert(distribution);
      for (size_t j = 0; j <= inputEnumeration->getNumElements(); ++j)
        scores->incrementValue(j, distribution->computeProbability(Variable(j, inputEnumeration)));
      scores->incrementValue(inputEnumeration->getNumElements() + 1, distribution->computeEntropy());
    }
  }

private:
  EnumerationPtr inputEnumeration;
};

// value sum, missing value count
class AccumulateDoubleContainerOperator : public AccumulateContainerOperator
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType)
    {return missingOrPresentEnumeration;}

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const DenseDoubleVectorPtr& scores = res->computeStep(i);
      Variable element = container->getElement(i);
      if (element.exists())
        scores->incrementValue(1, element.getDouble());
      else
        scores->incrementValue(0, 1.0);
    }
  }
};

class AccumulateDoubleVectorContainerOperator : public AccumulateContainerOperator
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr doubleVectorType)
  {
    EnumerationPtr featuresEnumeration;
    TypePtr featuresType;
    if (!DoubleVector::getTemplateParameters(context, doubleVectorType, featuresEnumeration, featuresType))
      return EnumerationPtr();
    return featuresEnumeration;
  }

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      DoubleVectorPtr vector = container->getElement(i).getObjectAndCast<DoubleVector>();
      jassert(vector);
      vector->addTo(res->computeStep(i));
    }
  }
};

class AccumulateOperator : public ProxyFunction
{
public:
  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    if (inputVariables.size() == 1 && inputVariables[0]->getType()->inheritsFrom(containerClass(anyType)))
    {
      TypePtr elementsType;
      if (getContainerElementsType(defaultExecutionContext(), inputVariables[0]->getType(), elementsType))
      {
        if (elementsType.dynamicCast<Enumeration>())
          return new AccumulateEnumerationContainerOperator();
        else if (elementsType->inheritsFrom(doubleType))
          return new AccumulateDoubleContainerOperator();
        else if (elementsType->inheritsFrom(enumerationDistributionClass(anyType)))
          return new AccumulateEnumerationDistributionContainerOperator();
        else if (elementsType->inheritsFrom(objectClass))
        {
          // todo: verify if the object only contains double members
          return new AccumulateDoubleVectorContainerOperator();
        }
      }
    }
    return FunctionPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_
