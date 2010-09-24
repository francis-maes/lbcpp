/*-----------------------------------------.---------------------------------.
| Filename: HardDiscretizedNumberFeatu...h | A Feature Generator that        |
| Author  : Julien Becker                  | discretized one feature.        |
| Started : 23/09/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{
  
class HardDiscretizedNumberFeatures : public Perception
{
public:
  HardDiscretizedNumberFeatures(TypePtr elementType = probabilityType(), size_t numIntervals = 10)
    : elementType(elementType), numIntervals(numIntervals) {}

  virtual TypePtr getInputType() const
    {return elementType;}

  virtual size_t getNumOutputVariables() const
    {return numIntervals;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
    {return T("Hard[") + String((int)index) + T("]");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    // Currently, only probabilityType is allowed. In future, we will add angleType or more generic doubleType
    jassert(input.getType()->inheritsFrom(probabilityType()));
    double value = input.getDouble();
    jassert(value > -0.001 && value < 1.001);
    value *= (double)numIntervals;
    size_t first = (size_t)value;
    
    if (first == numIntervals)
      --first;
    
    callback->sense(first, 1.0);
  }
  
private:
  friend class HardDiscretizedNumberFeaturesClass;
  
  TypePtr elementType;
  size_t numIntervals;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
