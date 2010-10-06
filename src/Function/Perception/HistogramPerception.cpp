/*-----------------------------------------.---------------------------------.
| Filename: HistogramPerception.cpp        | Histogram Perception            |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "HistogramPerception.h"
#include <lbcpp/Data/Cache.h>
using namespace lbcpp;

namespace lbcpp
{

class AccumulatedScores : public Object
{
public:
  void compute(ContainerPtr container)
  {
    jassert(container);

    TypePtr type = container->getElementsType();
    size_t n = container->getNumElements();
    accumulators.resize(n);

    EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
    if (enumeration)
      computeForEnumeration(enumeration, container);
    else if (type->inheritsFrom(doubleType()))
      computeForDouble(container);
    else if (type->inheritsFrom(discreteProbabilityDistributionClass(anyType())))
    {
      EnumerationPtr enumeration = type->getTemplateArgument(0).dynamicCast<Enumeration>();
      jassert(enumeration);
      computeForDiscreteDistribution(enumeration, container);
    }
    else
      jassert(false);
  }

  std::vector<double>& getAccumulatedScores(size_t index)
    {jassert(index < accumulators.size()); return accumulators[index];}

  size_t getNumElements() const
    {return accumulators.size();}

  juce_UseDebuggingNewOperator

private:
  std::vector< std::vector<double> > accumulators; // index -> label -> count

  void computeForEnumeration(EnumerationPtr enumeration, ContainerPtr container)
  {
    accumulators[0].resize(enumeration->getNumElements() + 1, 0.0);
    for (size_t i = 0; i < accumulators.size(); ++i)
    {
      std::vector<double>& scores = accumulators[i];
      if (i > 0)
        scores = accumulators[i - 1];
      scores[container->getElement(i).getInteger()] += 1.0;
    }
  }

  void computeForDouble(ContainerPtr container)
  {
    accumulators[0].resize(2, 0.0);
    for (size_t i = 0; i < accumulators.size(); ++i)
    {
      std::vector<double>& scores = accumulators[i];
      if (i > 0)
        scores = accumulators[i - 1];
      Variable element = container->getElement(i);
      if (element)
        scores[0] += element.getDouble();
      else
        scores[1] += 1.0;
    }
  }

  void computeForDiscreteDistribution(EnumerationPtr enumeration, ContainerPtr container)
  {
    accumulators[0].resize(enumeration->getNumElements() + 1, 0.0);
    for (size_t i = 0; i < accumulators.size(); ++i)
    {
      std::vector<double>& scores = accumulators[i];
      if (i > 0)
        scores = accumulators[i - 1];
      DiscreteProbabilityDistributionPtr distribution = container->getElement(i).getObjectAndCast<DiscreteProbabilityDistribution>();
      jassert(distribution);
      for (size_t j = 0; j <= enumeration->getNumElements(); ++j)
        scores[j] += distribution->compute(Variable(j, enumeration));
    }
  }
};

typedef ReferenceCountedObjectPtr<AccumulatedScores> AccumulatedScoresPtr;

class AccumulatedScoresCache : public Cache
{
public:
  // Every 30 insertions, prune scores that have not been accessed during the last minute
  AccumulatedScoresCache() : Cache(30, 60.0) {}

  juce_UseDebuggingNewOperator

protected:
  virtual Variable createEntry(ObjectPtr object) const
  {
    return new AccumulatedScores();
  }
};



}; /* namespace lbcpp */

HistogramPerception::HistogramPerception(TypePtr elementsType, bool useCache)
  : elementsType(elementsType)
{
  if (useCache)
    cache = new AccumulatedScoresCache();
}

size_t HistogramPerception::getNumOutputVariables() const
{
  EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();
  if (enumeration)
    return enumeration->getNumElements() + 2;
  if (elementsType->inheritsFrom(doubleType()))
    return 3;
  if (elementsType->inheritsFrom(discreteProbabilityDistributionClass(anyType())))
    return elementsType->getTemplateArgument(0).dynamicCast<Enumeration>()->getNumElements() + 2;
  jassert(false);
  return 0;
}

TypePtr HistogramPerception::getOutputVariableType(size_t index) const
{
  if (index == 0 && elementsType->inheritsFrom(doubleType()))
    return elementsType;
  return index == getNumOutputVariables() - 1 ? negativeLogProbabilityType() : probabilityType();
}

String HistogramPerception::getOutputVariableName(size_t index) const
{
  if (index == getNumOutputVariables() - 1)
    return T("entropy");
  else if (index == getNumOutputVariables() - 2)
    return T("p[missing]");

  EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();
  if (enumeration)
    return T("p[") + enumeration->getElementName(index) + T("]");
  if (elementsType->inheritsFrom(doubleType()))
    return T("average");
  if (elementsType->inheritsFrom(discreteProbabilityDistributionClass(anyType())))
  {
    enumeration = elementsType->getTemplateArgument(0).dynamicCast<Enumeration>();
    return T("p[") + enumeration->getElementName(index) + T("]");
  }
  jassert(false);
  return String::empty;
}

void HistogramPerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  ContainerPtr container = input[0].getObjectAndCast<Container>();
  Variable indicesPair = input[1];
  if (!container)
    return;

  size_t n = container->getNumElements();
  size_t startPosition = juce::jlimit(0, (int)n, indicesPair[0].getInteger());
  size_t endPosition = juce::jlimit(0, (int)n, indicesPair[1].getInteger());
  jassert(endPosition >= startPosition);

  AccumulatedScoresPtr scores;
  if (cache)
  {
    scores = cache->getOrCreateEntryAndCast<AccumulatedScores>(container);
    if (!scores->getNumElements())
      scores->compute(container);
  }
  else
  {
    scores = new AccumulatedScores();
    scores->compute(container);
  }

  const std::vector<double>& startScores = scores->getAccumulatedScores(startPosition);
  const std::vector<double>& endScores = scores->getAccumulatedScores(endPosition - 1);

  // FIXME! This Perception should output a DiscreteProbabilityDistribution directly
  double invK = 1.0 / (endPosition - startPosition - 1.0);
  double entropy = 0.0;
  for (size_t i = 0; i < startScores.size(); ++i)
  {
    double p = (endScores[i] - startScores[i]) * invK;
    callback->sense(i, Variable(p, getOutputVariableType(i)));
    if (p)
      entropy -= p * log2(p);
  }
  callback->sense(startScores.size(), Variable(entropy, negativeLogProbabilityType()));
}
