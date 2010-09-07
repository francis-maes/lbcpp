/*-----------------------------------------.---------------------------------.
| Filename: HistogramPerception.h          | Histogram Perception            |
| Author  : Julien Becker                  |                                 |
| Started : 02/09/2010 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_HISTOGRAM_H_
# define LBCPP_DATA_PERCEPTION_HISTOGRAM_H_

# include <lbcpp/Function/Perception.h>
# include <lbcpp/Data/ProbabilityDistribution.h>

namespace lbcpp
{

class AccumulatedScores;
typedef ReferenceCountedObjectPtr<AccumulatedScores> AccumulatedScoresPtr;

class HistogramPerception : public Perception
{
public:
  HistogramPerception(TypePtr elementsType, bool useCache)
    : elementsType(elementsType), useCache(useCache), accumulators(NULL) {}
  HistogramPerception() : useCache(false), accumulators(NULL) {}
  virtual ~HistogramPerception();

  virtual TypePtr getInputType() const
    {return pairType(vectorClass(elementsType), pairType(integerType(), integerType()));}

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

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

protected:
  friend class HistogramPerceptionClass;

  TypePtr elementsType;
  bool useCache;

  AccumulatedScores* accumulators;
  VectorPtr previousVector;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_HISTOGRAM_H_
