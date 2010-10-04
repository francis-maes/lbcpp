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
# include "HardDiscretizedNumberFeatures.h"

namespace lbcpp
{
  
class SoftDiscretizedNumberFeatures : public DiscretizedNumberFeatures
{
public:
  SoftDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures, bool cyclicBehavior)
    : DiscretizedNumberFeatures(inputType, minimumValue, maximumValue, numIntervals, doOutOfBoundsFeatures), cyclicBehavior(cyclicBehavior)
  {
    jassert(!cyclicBehavior || !doOutOfBoundsFeatures); // out-of-bounds features are never active when using the cyclicBehavior
  }
  SoftDiscretizedNumberFeatures() : cyclicBehavior(false) {}
  
  virtual size_t getNumOutputVariables() const
    {return (doOutOfBoundsFeatures ? 2 : 0) + numIntervals + (cyclicBehavior ? 0 : 1);}
  
  virtual String getOutputVariableName(size_t index) const
  {
    String res = getOutOfBoundsFeatureName(index);
    if (res.isNotEmpty())
      return res;
    return T("close to ") + String(getBoundary(index));
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    double value = getValue(input);
    double halfWidth = (maximumValue - minimumValue) / numIntervals;

    if (value <= minimumValue - halfWidth)
    {
      if (doOutOfBoundsFeatures)
        callback->sense(0, 1.0);
    }
    else if (value <= minimumValue)
    {
      if (doOutOfBoundsFeatures)
      {
        double k = (minimumValue - value) / halfWidth;
        callback->sense(0, k);
        callback->sense(2, 1 - k); // first interval feature
      }
    }
    else if (value <= maximumValue)
    {
      int discretizedValue = (int)(numIntervals * (value - minimumValue) / (maximumValue - minimumValue));
      if (discretizedValue == numIntervals)
        --discretizedValue;
      jassert(discretizedValue >= 0 && discretizedValue < (int)numIntervals);

      size_t variable1 = (size_t)discretizedValue;
      double k = (value - getBoundary(variable1)) / halfWidth;
      size_t variable2 = variable1 + 1;
      
      if (cyclicBehavior && variable2 == numIntervals)
        variable2 = 0;

      if (k != 1.0)
        callback->sense((doOutOfBoundsFeatures ? 2 : 0) + variable1, 1.0 - k);
      if (k != 0.0)
        callback->sense((doOutOfBoundsFeatures ? 2 : 0) + variable2, k);
    }
    else if (value <= maximumValue + halfWidth)
    {
      if (doOutOfBoundsFeatures)
      {
        double k = (value - maximumValue) / halfWidth;
        callback->sense(1, k);
        callback->sense(2 + numIntervals, 1 - k); // last interval feature
      }
    }
    else
    {
      if (doOutOfBoundsFeatures)
        callback->sense(1, 1.0);
    }
  }
  
private:
  friend class SoftDiscretizedNumberFeaturesClass;
  
  bool cyclicBehavior;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_
