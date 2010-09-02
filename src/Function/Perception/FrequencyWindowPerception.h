/*-----------------------------------------.---------------------------------.
| Filename: FrequencyWindowPerception.h    | Frequency Window Perception     |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_FREQUENCY_WINDOW_H_
# define LBCPP_DATA_PERCEPTION_FREQUENCY_WINDOW_H_

# include <lbcpp/Function/Perception.h>
# include <lbcpp/Data/ProbabilityDistribution.h>

namespace lbcpp
{

class FrequencyWindowPerception : public Perception
{
public:
  FrequencyWindowPerception(TypePtr elementsType, size_t windowSize)
    : elementsType(elementsType), windowSize(windowSize) {}
  FrequencyWindowPerception() : windowSize(0) {}

  virtual TypePtr getInputType() const
    {return pairType(vectorClass(elementsType), integerType());}

  virtual TypePtr getOutputType() const
    {return Perception::getOutputType();}

  virtual size_t getNumOutputVariables() const
  {
    EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();
    if (enumeration)
      return enumeration->getNumElements() + 1;
    if (elementsType->inheritsFrom(doubleType()))
      return 1;
    if (elementsType->inheritsFrom(discreteProbabilityDistributionClass(anyType())))
      return elementsType->getTemplateArgument(0).dynamicCast<Enumeration>()->getNumElements();
    jassert(false);
    return 0;
  }

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
  {
    EnumerationPtr enumeration = elementsType.dynamicCast<Enumeration>();
    if (enumeration)
      return (index == enumeration->getNumElements()) ? "Missing" : enumeration->getElementName(index);
    if (elementsType->inheritsFrom(doubleType()))
      return T("average");
    if (elementsType->inheritsFrom(discreteProbabilityDistributionClass(anyType())))
      return elementsType->getTemplateArgument(0).dynamicCast<Enumeration>()->getElementName(index);
    jassert(false);
    return T("undefined");
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    VectorPtr vector = input[0].getObjectAndCast<Vector>();
    if (!vector)
      return;
    
    AccumulatedScoresVectorPtr accumulators = vector->getAccumulatedScores();
    size_t n = accumulators->getNumElements();
    size_t startPosition = 0;
    size_t endPosition = n;
    if (windowSize)
    {
      int position = input[1].getInteger() - (int)(windowSize / 2);
      startPosition = juce::jlimit(0, (int)n, position); // inclusive
      endPosition = juce::jlimit(0, (int)n, position + (int)windowSize); // exclusive
    }
    
    const std::vector<double>& startScores = accumulators->getAccumulatedScores(startPosition);
    const std::vector<double>& endScores = accumulators->getAccumulatedScores(endPosition - 1);
    
    for (size_t i = 0; i < startScores.size(); ++i)
      callback->sense(i, Variable((endScores[i] - startScores[i]) / (endPosition - startPosition), doubleType()));
  }

protected:
  friend class FrequencyWindowPerceptionClass;

  TypePtr elementsType;
  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_FREQUENCY_WINDOW_H_
