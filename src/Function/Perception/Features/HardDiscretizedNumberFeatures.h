/*-----------------------------------------.---------------------------------.
| Filename: HardDiscretizedNumberFeatu...h | Discretize a continuous number  |
| Author  : Julien Becker                  | into intervals                  |
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
  HardDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : inputType(inputType), minimumValue(minimumValue), maximumValue(maximumValue), numIntervals(numIntervals), doOutOfBoundsFeatures(doOutOfBoundsFeatures)
    {jassert(maximumValue > minimumValue);}

  HardDiscretizedNumberFeatures() : numIntervals(0) {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual size_t getNumOutputVariables() const
    {return (doOutOfBoundsFeatures ? 2 : 0) + numIntervals;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
  {
    if (doOutOfBoundsFeatures)
    {
      if (index == 0)
        return T("less than ") + String(minimumValue);
      else if (index == 1)
        return T("more than ") + String(maximumValue);
      else
        index -= 2;
    }

    String res(T("in ["));
    res += String(getBoundary(index)) + T(", ") + String(getBoundary(index + 1));
    res += index == numIntervals - 1 ? T("]") : T("[");
    return res;
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input);
    double value = input.isDouble() ? input.getDouble() : (double)input.getInteger();
    if (value < minimumValue)
    {
      if (doOutOfBoundsFeatures)
        callback->sense(0, 1.0);
    }
    else if (value <= maximumValue)
    {
      size_t discretizedValue = (size_t)(numIntervals * (value - minimumValue) / (maximumValue - minimumValue));
      if (discretizedValue == numIntervals)
        --discretizedValue;
      callback->sense((doOutOfBoundsFeatures ? 2 : 0) + discretizedValue, 1.0);
    }
    else
    {
      if (doOutOfBoundsFeatures)
        callback->sense(1, 1.0);
    }
  }
  
private:
  friend class HardDiscretizedNumberFeaturesClass;
  
  TypePtr inputType;
  double minimumValue;
  double maximumValue;
  size_t numIntervals;
  bool doOutOfBoundsFeatures;

  double getBoundary(size_t index) const
  {
    jassert(index <= numIntervals);
    return minimumValue + (maximumValue - minimumValue) * (double)index / numIntervals;
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
