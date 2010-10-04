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
  
class DiscretizedNumberFeatures : public Perception
{
public:
  DiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : inputType(inputType), minimumValue(minimumValue), maximumValue(maximumValue), numIntervals(numIntervals), doOutOfBoundsFeatures(doOutOfBoundsFeatures)
  {
    jassert(maximumValue > minimumValue);
    jassert(numIntervals > 1);
  }

  DiscretizedNumberFeatures()
    : minimumValue(0.0), maximumValue(0.0), numIntervals(0), doOutOfBoundsFeatures(false) {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

protected:
  friend class DiscretizedNumberFeaturesClass;

  TypePtr inputType;
  double minimumValue;
  double maximumValue;
  size_t numIntervals;
  bool doOutOfBoundsFeatures;

  double getValue(const Variable& input) const
    {jassert(input); return input.isDouble() ? input.getDouble() : (double)input.getInteger();}

  double getBoundary(int index) const
  {
    jassert(index >= -1 && index <= (int)numIntervals + 1);
    return minimumValue + (maximumValue - minimumValue) * (double)index / numIntervals;
  }

  String getOutOfBoundsFeatureName(size_t& index) const
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
    return String::empty;
  }
};

class HardDiscretizedNumberFeatures : public DiscretizedNumberFeatures
{
public:
  HardDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : DiscretizedNumberFeatures(inputType, minimumValue, maximumValue, numIntervals, doOutOfBoundsFeatures) {}

  HardDiscretizedNumberFeatures() {}

  virtual size_t getNumOutputVariables() const
    {return (doOutOfBoundsFeatures ? 2 : 0) + numIntervals;}

  virtual String getOutputVariableName(size_t index) const
  {
    String res = getOutOfBoundsFeatureName(index);
    if (res.isEmpty())
    {
      res = T("in [") + String(getBoundary(index)) + T(", ") + String(getBoundary(index + 1));
      res += index == numIntervals - 1 ? T("]") : T("[");
    }
    return res;
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    double value = getValue(input);
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
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
