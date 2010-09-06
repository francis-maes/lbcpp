/*-----------------------------------------.---------------------------------.
| Filename: HistogramPerception.cpp        | Histogram Perception            |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "HistogramPerception.h"
using namespace lbcpp;

namespace lbcpp
{

class AccumulatedScores
{
public:
  AccumulatedScores(const VectorPtr vector) : accumulators(vector->getNumElements())
  {
    computeAccumulatedScores(vector);
  }
  
  std::vector<double>& getAccumulatedScores(size_t index)
    {jassert(index < accumulators.size()); return accumulators[index];}
  
  size_t getNumElements() const
    {return accumulators.size();}

private:
  void computeAccumulatedScores(const VectorPtr vector)
  {    
    TypePtr type = vector->getElementsType();
    size_t n = vector->getNumElements();
    EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
    if (enumeration)
    {
      accumulators[0].resize(enumeration->getNumElements() + 1, 0.0);
      
      for (size_t i = 0; i < n; ++i)
      {
        std::vector<double>& scores = accumulators[i];
        if (i > 0)
          scores = accumulators[i - 1];
        scores[vector->getElement(i).getInteger()] += 1.0;
      }
      return;
    }
    
    if (type->inheritsFrom(doubleType()))
    {
      accumulators[0].resize(1, 0.0);
      
      for (size_t i = 0; i < n; ++i)
      {
        std::vector<double>& scores = accumulators[i];
        if (i > 0)
          scores = accumulators[i - 1];
        if (vector->getElement(i))
          scores[0] += vector->getElement(i).getDouble();
      }
      return;
    }
    
    if (type->inheritsFrom(discreteProbabilityDistributionClass(anyType())))
    {
      EnumerationPtr enumeration = type->getTemplateArgument(0).dynamicCast<Enumeration>();
      jassert(enumeration);
      
      accumulators[0].resize(enumeration->getNumElements(), 0.0); // No missing value allowed
      
      for (size_t i = 0; i < n; ++i)
      {
        std::vector<double>& scores = accumulators[i];
        if (i > 0)
          scores = accumulators[i - 1];
        for (size_t j = 0; j < enumeration->getNumElements(); ++j)
          scores[j] += vector->getElement(i).getObjectAndCast<DiscreteProbabilityDistribution>()->compute(Variable(j, enumeration));
      }
      return;
    }
    
    jassert(false);
  }
  
private:
  std::vector<std::vector<double> > accumulators; // index -> label -> count
};

}; /* namespace lbcpp */

void HistogramPerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  VectorPtr vector = input[0].getObjectAndCast<Vector>();
  if (!vector)
    return;

  if (!useCache || vector != previousVector)
    accumulators = new AccumulatedScores(vector);
  previousVector = vector;

  size_t n = vector->getNumElements();
  size_t startPosition = juce::jlimit(0, (int)n, input[1][0].getInteger());
  size_t endPosition = juce::jlimit(0, (int)n, input[1][1].getInteger());
  jassert(endPosition >= startPosition);
  const std::vector<double>& startScores = accumulators->getAccumulatedScores(startPosition);
  const std::vector<double>& endScores = accumulators->getAccumulatedScores(endPosition - 1);

  for (size_t i = 0; i < startScores.size(); ++i)
    callback->sense(i, Variable((endScores[i] - startScores[i]) / (endPosition - startPosition), doubleType()));
}

