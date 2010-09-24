/*-----------------------------------------.---------------------------------.
 | Filename: SoftDiscretizedNumberFeatu...h | A Feature Generator that        |
 | Author  : Julien Becker                  | discretized one feature.        |
 | Started : 24/09/2010 10:26               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{
  
class SoftDiscretizedNumberFeatures : public Perception
{
public:
  SoftDiscretizedNumberFeatures(TypePtr elementType = probabilityType(), size_t numIntervals = 10, bool cycle = false)
  : elementType(elementType), numIntervals(numIntervals) {}
  
  virtual TypePtr getInputType() const
    {return elementType;}
  
  virtual size_t getNumOutputVariables() const
    {return numIntervals;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("Soft[") + String((int)index) + T("]");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    // Currently, only probabilityType is allowed. In future, we will add angleType or more generic doubleType
    jassert(input.getType()->inheritsFrom(probabilityType()));
    double value = input.getDouble();
    jassert(value > -0.001 && value < 1.001);
    value *= numIntervals - 1;
    size_t first = (size_t)value;
    size_t second = first + 1;
    
    if (second == numIntervals && cycle)
      second = 0;
    double k = value - first;
    if (k != 1.0)
      callback->sense(first, 1.0 - k);
    if (k != 0.0 && second < numIntervals)
      callback->sense(second, k);
  }
  
private:
  friend class SoftDiscretizedNumberFeaturesClass;
  
  TypePtr elementType;
  size_t numIntervals;
  bool cycle;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_
