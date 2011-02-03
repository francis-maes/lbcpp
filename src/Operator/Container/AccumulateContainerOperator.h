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
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

extern ClassPtr cumulativeScoreVectorClass(TypePtr scoresEnumeration);

class CumulativeScoreVector : public Container
{
public:
  CumulativeScoreVector(ClassPtr thisClass, EnumerationPtr scores, size_t size)
    : Container(thisClass), scores(scores), accumulators(size), elementsType(enumBasedDoubleVectorClass(scores))
  {
    accumulators[0].resize(scores->getNumElements(), 0.0);
  }

  CumulativeScoreVector() {}

  std::vector<double>& getAccumulatedScores(size_t index)
    {jassert(index < accumulators.size()); return accumulators[index];}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual size_t getNumElements() const
    {return accumulators.size();}

  virtual Variable getElement(size_t index) const
    {return new DenseDoubleObject(elementsType, accumulators[index]);}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

  std::vector<double>& computeStep(size_t i)
  {
   std::vector<double>& scores = accumulators[i];
    if (i > 0)
      scores = accumulators[i - 1];
    return scores;
  }

  lbcpp_UseDebuggingNewOperator

private:
  EnumerationPtr scores;
  std::vector< std::vector<double> > accumulators; // index -> label -> count
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
      std::vector<double>& scores = res->computeStep(i);
      scores[container->getElement(i).getInteger()] += 1.0;
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
      std::vector<double>& scores = res->computeStep(i);
      
      EnumerationDistributionPtr distribution = container->getElement(i).getObjectAndCast<EnumerationDistribution>();
      jassert(distribution);
      for (size_t j = 0; j <= inputEnumeration->getNumElements(); ++j)
        scores[j] += distribution->computeProbability(Variable(j, inputEnumeration));
      scores.back() += distribution->computeEntropy();
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
      std::vector<double>& scores = res->computeStep(i);
      Variable element = container->getElement(i);
      if (element.exists())
        scores[1] += element.getDouble();
      else
        scores[0] += 1.0;
    }
  }
};

class AccumulateDoubleObjectContainerOperator : public AccumulateContainerOperator
{
public:
  virtual EnumerationPtr getScoresEnumeration(ExecutionContext& context, TypePtr elementsType)
    {return variablesEnumerationEnumeration(elementsType);}

  virtual void accumulate(const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<double>& scores = res->computeStep(i);
   
      ObjectPtr object = container->getElement(i).getObject();
      SparseDoubleObjectPtr sparseDoubleObject = object.dynamicCast<SparseDoubleObject>();
      if (sparseDoubleObject)
      {
        const std::vector<std::pair<size_t, double> >& features = sparseDoubleObject->getValues();
        for (size_t i = 0; i < features.size(); ++i)
          scores[features[i].first] += features[i].second;
      }
      else
      {
        // not implemented
        jassert(false);
      }
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
          return new AccumulateDoubleObjectContainerOperator();
        }
      }
    }
    return FunctionPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_
