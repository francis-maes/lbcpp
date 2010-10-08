/*-----------------------------------------.---------------------------------.
| Filename: HistogramPerception.h          | Histogram Perception            |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_HISTOGRAM_H_
# define LBCPP_FUNCTION_PERCEPTION_HISTOGRAM_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Data/ProbabilityDistribution.h>
# include <lbcpp/Data/Cache.h>

namespace lbcpp
{

class AccumulatedScoresCache : public Cache
{
public:
  AccumulatedScoresCache();

  juce_UseDebuggingNewOperator

protected:
  virtual Variable createEntry(ObjectPtr object) const;
};

class HistogramPerception : public Perception
{
public:
  HistogramPerception(TypePtr elementsType, bool useCache);
  HistogramPerception() {}

  virtual void getInput(const Variable& input, ContainerPtr& container, int& beginIndex, int& endIndex) const = 0;
  
  virtual String toString() const
    {return elementsType->getName() + T(" histogram");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

  juce_UseDebuggingNewOperator

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
  
  virtual void getInput(const Variable& input, ContainerPtr& container, int& beginIndex, int& endIndex) const
  {
    container = input.getObjectAndCast<Container>();
    if (container)
    {
      beginIndex = 0;
      endIndex = (int)container->getNumElements();
    }
  }
};

class WindowHistogramPerception : public HistogramPerception
{
public:
  WindowHistogramPerception(TypePtr elementsType, size_t windowSize, bool useCache)
    : HistogramPerception(elementsType, useCache), windowSize(windowSize) {}
  WindowHistogramPerception() : windowSize(0) {}

  virtual TypePtr getInputType() const
    {return pairClass(containerClass(elementsType), positiveIntegerType());}
  
  virtual void getInput(const Variable& input, ContainerPtr& container, int& beginIndex, int& endIndex) const
  {
    jassert(windowSize);
    container = input[0].getObjectAndCast<Container>();
    beginIndex = input[1].getInteger() - (int)(windowSize / 2);
    endIndex = beginIndex + windowSize;
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
    {return pairClass(containerClass(elementsType), pairClass(positiveIntegerType(), positiveIntegerType()));}
  
  virtual void getInput(const Variable& input, ContainerPtr& container, int& beginIndex, int& endIndex) const
  {
    container = input[0].getObjectAndCast<Container>();
    Variable indexPair = input[1];
    beginIndex = indexPair[0].getInteger();
    endIndex = indexPair[1].getInteger();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_HISTOGRAM_H_
