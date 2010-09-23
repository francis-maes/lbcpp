/*-----------------------------------------.---------------------------------.
| Filename: HardDiscretizedNumberFeatu...h | A Feature Generator that        |
| Author  : Francis Maes                   | discretized one feature.        |
| Started : 15/09/2010 19:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{
  
class HardDiscretizedNumberFeature : public Perception
{
public:
  HardDiscretizedNumberFeature(TypePtr elementType = probabilityType(), size_t numIntervals = 10)
    : elementType(elementType), numIntervals(numIntervals) {}

  virtual TypePtr getInputType() const
    {return elementType;}

  virtual size_t getNumOutputVariables() const
    {return numIntervals;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
    {return T("[") + String((int)index) + T("]");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.getType()->inheritsFrom(probabilityType()));
    double value = input.getDouble();
    jassert(value > -0.001 && value < 1.001);
    for (size_t i = 0; i < numIntervals; ++i)
      if (value < (i + 1) * 1.0 / numIntervals)
      {
        callback->sense(i, 1.0);
        break;
      }
  }
  
private:
  friend class HardDiscretizedNumberFeatureClass;
  
  TypePtr elementType;
  size_t numIntervals;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
