/*-----------------------------------------.---------------------------------.
| Filename: HardDiscretizedNumberFeatu...h | Discretize a continuous number  |
| Author  : Julien Becker                  | into intervals                  |
| Started : 23/09/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_HARD_DISCRETIZED_NUMBER_H_

# include "DiscretizedNumberFeatures.h"

namespace lbcpp
{

class HardDiscretizedNumberFeatures : public DiscretizedNumberFeatures
{
public:
  HardDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : DiscretizedNumberFeatures(inputType, minimumValue, maximumValue, numIntervals, doOutOfBoundsFeatures)
    {computeOutputType();}

  HardDiscretizedNumberFeatures() {}

  virtual void computeOutputType()
  {
    reserveOutputVariables((doOutOfBoundsFeatures ? 2 : 0) + numIntervals);
    if (doOutOfBoundsFeatures)
    {
      addOutputVariable(T("after ") + getBoundaryName(0), doubleType());
      addOutputVariable(T("after ") + getBoundaryName(numIntervals), doubleType());
    }
    for (size_t i = 0; i < numIntervals; ++i)
    {
      String name = T("in [") + getBoundaryName(i) + T(", ") + getBoundaryName(i + 1);
      name += i == numIntervals - 1 ? T("]") : T("[");
      addOutputVariable(name, doubleType());
    }
    Perception::computeOutputType();
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
