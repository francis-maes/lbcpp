/*-----------------------------------------.---------------------------------.
| Filename: AccumulateContainerOperator.h  | Accumulate Container Function   |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_
# define LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

extern ClassPtr cumulativeScoreVectorClass(TypePtr scoresEnumeration);

class CumulativeScoreVector : public Container
{
public:
  CumulativeScoreVector(EnumerationPtr scores)
    : Container(cumulativeScoreVectorClass(scores)), scores(scores) {}
  CumulativeScoreVector() {}

  std::vector<double>& getAccumulatedScores(size_t index)
    {jassert(index < accumulators.size()); return accumulators[index];}

  virtual size_t getNumElements() const
    {return accumulators.size();}

  virtual Variable getElement(size_t index) const
    {return new DenseDoubleObject(enumBasedDoubleVectorClass(scores), accumulators[index]);}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

  void beginCompute(size_t length)
  {
    accumulators.clear();
    accumulators.resize(length);
    accumulators[0].resize(scores->getNumElements());
  }

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
};

typedef ReferenceCountedObjectPtr<CumulativeScoreVector> CumulativeScoreVectorPtr;

class AccumulateContainerOperator : public Function
{
public:
  AccumulateContainerOperator(TypePtr elementsType)
    : elementsType(elementsType) {}
  AccumulateContainerOperator() {}

  virtual TypePtr getInputType() const
    {return containerClass(elementsType);}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return cumulativeScoreVectorClass(inputType);}

  virtual EnumerationPtr getScoresEnumeration() const = 0;
  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!checkType(context, input, containerClass(anyType)) || !checkExistence(context, input))
      return Variable();

    const ContainerPtr& container = input.getObjectAndCast<Container>();
    CumulativeScoreVectorPtr res(new CumulativeScoreVector(getScoresEnumeration()));
    accumulate(context, container, res);
    return res;
  };

protected:
  friend class AccumulateContainerOperatorClass;

  TypePtr elementsType;
};

// enum values + missing
class AccumulateEnumerationContainerOperator : public AccumulateContainerOperator
{
public:
  AccumulateEnumerationContainerOperator(EnumerationPtr enumeration)
    : AccumulateContainerOperator(enumeration) {}
  AccumulateEnumerationContainerOperator() {}

  virtual EnumerationPtr getScoresEnumeration() const
  {
    const EnumerationPtr& enumeration = elementsType.staticCast<Enumeration>();
    return addMissingToEnumerationEnumeration(enumeration);
  }

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    res->beginCompute(n);
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
  AccumulateEnumerationDistributionContainerOperator(EnumerationPtr enumeration)
    : AccumulateContainerOperator(enumerationDistributionClass(enumeration)), enumeration(enumeration) {}
  AccumulateEnumerationDistributionContainerOperator() {}

  virtual EnumerationPtr getScoresEnumeration() const
    {return addEntropyToEnumerationEnumeration(addMissingToEnumerationEnumeration(enumeration));}

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    res->beginCompute(n);
    for (size_t i = 0; i < n; ++i)
    {
      std::vector<double>& scores = res->computeStep(i);
      
      EnumerationDistributionPtr distribution = container->getElement(i).getObjectAndCast<EnumerationDistribution>();
      jassert(distribution);
      for (size_t j = 0; j <= enumeration->getNumElements(); ++j)
        scores[j] += distribution->compute(context, Variable(j, enumeration));
      scores.back() += distribution->computeEntropy();
    }
  }

protected:
  friend class AccumulateEnumerationDistributionContainerOperatorClass;

  EnumerationPtr enumeration;
};

// value sum, missing value count
class AccumulateDoubleContainerOperator : public AccumulateContainerOperator
{
public:
  AccumulateDoubleContainerOperator() : AccumulateContainerOperator(doubleType) {}
  
  virtual EnumerationPtr getScoresEnumeration() const
    {return missingOrPresentEnumeration;}

  virtual void accumulate(ExecutionContext& context, const ContainerPtr& container, const CumulativeScoreVectorPtr& res) const
  {
    size_t n = container->getNumElements();
    res->beginCompute(n);
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

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OPERATOR_ACCUMULATE_CONTAINER_H_
