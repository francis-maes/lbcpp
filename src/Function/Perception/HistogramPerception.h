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

class HistogramPerception : public Perception
{
public:
  HistogramPerception(TypePtr elementsType, bool useCache);
  HistogramPerception() {}

  virtual String getPreferedOutputClassName() const
    {return elementsType->getName() + T(" histogram");}

  virtual TypePtr getInputType() const
    {return pairType(vectorClass(elementsType), pairType(integerType(), integerType()));}

  virtual size_t getNumOutputVariables() const;

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const;

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const;

  juce_UseDebuggingNewOperator

protected:
  friend class HistogramPerceptionClass;

  TypePtr elementsType;
  CachePtr cache;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_HISTOGRAM_H_
