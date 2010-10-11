/*-----------------------------------------.---------------------------------.
| Filename: HistogramPerception.cpp        | Histogram Perception            |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "HistogramPerception.h"
using namespace lbcpp;

namespace lbcpp {

class AccumulatedScores : public Object
{
public:
  void compute(ContainerPtr container)
  {
    ScopedLock _(lock);
    jassert(container);

    TypePtr type = container->getElementsType();
    size_t n = container->getNumElements();
    accumulators.resize(n);

    EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
    if (enumeration)
      computeForEnumeration(enumeration, container);
    else if (type->inheritsFrom(doubleType))
      computeForDouble(container);
    else if (type->inheritsFrom(discreteProbabilityDistributionClass(anyType)))
    {
      EnumerationPtr enumeration = type->getTemplateArgument(0).dynamicCast<Enumeration>();
      jassert(enumeration);
      computeForDiscreteDistribution(enumeration, container);
    }
    else
      jassert(false);
  }

  CriticalSection lock;

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
      if (element.exists())
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

}; /* namespace lbcpp */

/*
** AccumulatedScoresCache
*/

// Every 30 insertions, prune scores that have not been accessed during the last minute
AccumulatedScoresCache::AccumulatedScoresCache()
  : Cache(30, 60.0) {}

Variable AccumulatedScoresCache::createEntry(ObjectPtr object) const
  {return new AccumulatedScores();}

/*
** HistogramPerception
*/
HistogramPerception::HistogramPerception(TypePtr elementsType, bool useCache)
  : elementsType(elementsType)
{
  if (useCache)
    cache = new AccumulatedScoresCache();
  computeOutputType();
}

void HistogramPerception::computeOutputType()
{
  EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();
  if (enumeration)
  {
    reserveOutputVariables(enumeration->getNumElements() + 2);
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      addOutputVariable(T("p[") + enumeration->getElementName(i) + T("]"), probabilityType);
  }
  else if (elementsType->inheritsFrom(doubleType))
  {
    reserveOutputVariables(3);
    addOutputVariable(T("average"), elementsType);
  }
  else if (elementsType->inheritsFrom(discreteProbabilityDistributionClass(anyType)))
  {
    enumeration = elementsType->getTemplateArgument(0).dynamicCast<Enumeration>();
    reserveOutputVariables(enumeration->getNumElements() + 2);
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      addOutputVariable(T("p[") + enumeration->getElementName(i) + T("]"), probabilityType);
  }

  addOutputVariable(T("p[missing]"), probabilityType);
  addOutputVariable(T("entropy"), negativeLogProbabilityType);
  Perception::computeOutputType();
}

void HistogramPerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  ContainerPtr container;
  int startPosition, endPosition;
  getInput(input, container, startPosition, endPosition);
  if (!container)
    return;

  size_t n = container->getNumElements();
  startPosition = juce::jlimit(0, (int)n, startPosition);
  endPosition = juce::jlimit(0, (int)n, endPosition);
  jassert(endPosition >= startPosition);

  AccumulatedScoresPtr scores;
  if (cache)
    scores = cache->getOrCreateEntryAndCast<AccumulatedScores>(container);
  else
    scores = new AccumulatedScores();

  scores->lock.enter();
  if (!scores->getNumElements())
    scores->compute(container);
  std::vector<double> startScores = scores->getAccumulatedScores(startPosition);
  std::vector<double> endScores = scores->getAccumulatedScores(endPosition - 1);
  scores->lock.exit();
  
  size_t numScores = startScores.size();

  // FIXME! This Perception should output a DiscreteProbabilityDistribution directly
  double invK = 1.0 / (endPosition - startPosition - 1.0);
  double entropy = 0.0;
  for (size_t i = 0; i < numScores; ++i)
  {
    double p = (endScores[i] - startScores[i]) * invK;
    callback->sense(i, p);
    if (p)
      entropy -= p * log2(p);
  }

  callback->sense(numScores, entropy);
}
