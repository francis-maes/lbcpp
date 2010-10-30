/*-----------------------------------------.---------------------------------.
| Filename: HistogramPerception.h          | Histogram Perception            |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_CONTAINER_HISTOGRAM_H_
# define LBCPP_FUNCTION_PERCEPTION_CONTAINER_HISTOGRAM_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Data/ProbabilityDistribution.h>
# include <lbcpp/Data/Cache.h>
# include <lbcpp/Data/Pair.h>

namespace lbcpp
{

class AccumulatedScoresCache : public Cache
{
public:
  AccumulatedScoresCache();

  lbcpp_UseDebuggingNewOperator

protected:
  virtual Variable createEntry(const ObjectPtr& object) const;
};

class HistogramPerception : public Perception
{
public:
  HistogramPerception(TypePtr elementsType, bool useCache);
  HistogramPerception() {}

  virtual const ContainerPtr& getInput(const Variable& input, int& beginIndex, int& endIndex) const = 0;
  
  virtual String toString() const
    {return elementsType->getName() + T(" histogram");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

  lbcpp_UseDebuggingNewOperator

protected:
  virtual void computeOutputType();

  friend class HistogramPerceptionClass;

  TypePtr elementsType;
  CachePtr cache;
};

class ContainerHistogramPerception : public HistogramPerception
{
public:
  ContainerHistogramPerception(TypePtr elementsType, bool useCache)
    : HistogramPerception(elementsType, useCache) {}
  ContainerHistogramPerception() {}

  virtual TypePtr getInputType() const
    {return containerClass(elementsType);}
  
  virtual const ContainerPtr& getInput(const Variable& input, int& beginIndex, int& endIndex) const
  {
    const ContainerPtr& container = input.getObjectAndCast<Container>();
    if (container)
    {
      beginIndex = 0;
      endIndex = (int)container->getNumElements();
    }
    return container;
  }
};

class WindowHistogramPerception : public HistogramPerception
{
public:
  WindowHistogramPerception(TypePtr elementsType, size_t windowSize, bool useCache)
    : HistogramPerception(elementsType, useCache), windowSize(windowSize) {}
  WindowHistogramPerception() : windowSize(0) {}

  virtual TypePtr getInputType() const
    {return pairClass(containerClass(elementsType), positiveIntegerType);}
  
  virtual const ContainerPtr& getInput(const Variable& input, int& beginIndex, int& endIndex) const
  {
    jassert(windowSize);
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    const ContainerPtr& container = pair->getFirst().getObjectAndCast<Container>();
    beginIndex = pair->getSecond().getInteger() - (int)(windowSize / 2);
    endIndex = beginIndex + (int)windowSize;
    return container;
  }

protected:
  friend class WindowHistogramPerceptionClass;

  size_t windowSize;
};

class SegmentHistogramPerception : public HistogramPerception
{
public:
  SegmentHistogramPerception(TypePtr elementsType, bool useCache)
    : HistogramPerception(elementsType, useCache) {}
  SegmentHistogramPerception() {}

  virtual TypePtr getInputType() const
    {return pairClass(containerClass(elementsType), pairClass(positiveIntegerType, positiveIntegerType));}
  
  virtual const ContainerPtr& getInput(const Variable& input, int& beginIndex, int& endIndex) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    const ContainerPtr& container = pair->getFirst().getObjectAndCast<Container>();
    const PairPtr& indexPair = pair->getSecond().getObjectAndCast<Pair>();
    beginIndex = indexPair->getFirst().getInteger();
    endIndex = indexPair->getSecond().getInteger(); 
    return container;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_CONTAINER_HISTOGRAM_H_
