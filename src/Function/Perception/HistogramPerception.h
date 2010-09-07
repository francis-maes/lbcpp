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

  virtual size_t getNumOutputVariables() const;

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const;

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
