/*-----------------------------------------.---------------------------------.
 | Filename: SoftDiscretizedNumberFeatu...h | A Feature Generator that        |
 | Author  : Julien Becker                  | discretized one feature.        |
 | Started : 24/09/2010 10:26               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_

# include "DiscretizedNumberFeatures.h"

namespace lbcpp
{

class SoftDiscretizedNumberFeatures : public DiscretizedNumberFeatures
{
public:
  SoftDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures, bool cyclicBehavior)
    : DiscretizedNumberFeatures(inputType, minimumValue, maximumValue, numIntervals, doOutOfBoundsFeatures), cyclicBehavior(cyclicBehavior)
  {
    jassert(!cyclicBehavior || !doOutOfBoundsFeatures); // out-of-bounds features are never active when using the cyclicBehavior
    computeOutputVariables();
  }

  SoftDiscretizedNumberFeatures() : cyclicBehavior(false) {}
   
  virtual void computeOutputVariables()
  {
    reserveOutputVariables((doOutOfBoundsFeatures ? 2 : 0) + numIntervals + (cyclicBehavior ? 0 : 1));
    if (doOutOfBoundsFeatures)
    {
      addOutputVariable(T("after ") + getBoundaryName(0), doubleType());
      addOutputVariable(T("after ") + getBoundaryName(numIntervals), doubleType());
    }
    for (size_t i = 0; i < numIntervals; ++i)
      addOutputVariable(T("close to ") + getBoundaryName(i), doubleType());
    if (!cyclicBehavior)
      addOutputVariable(T("close to ") + getBoundaryName(numIntervals), doubleType());
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
      if (discretizedValue == (int)numIntervals)
        --discretizedValue;
      jassert(discretizedValue >= 0 && discretizedValue < (int)numIntervals);

      size_t variable1 = (size_t)discretizedValue;
      double k = (value - getBoundary(variable1)) / halfWidth;
      size_t variable2 = variable1 + 1;
      
      if (cyclicBehavior && variable2 == numIntervals)
        variable2 = 0;

      static const double epsilon = 1e-09;
      if (k < 1.0 - epsilon)
        callback->sense((doOutOfBoundsFeatures ? 2 : 0) + variable1, 1.0 - k);
      if (k > epsilon)
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
 
class SoftDiscretizedLogNumberFeatures : public SoftDiscretizedNumberFeatures
{
public:
  SoftDiscretizedLogNumberFeatures(TypePtr inputType, double minimumLogValue, double maximumLogValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : SoftDiscretizedNumberFeatures(inputType, minimumLogValue, maximumLogValue, numIntervals, doOutOfBoundsFeatures, false) {}

  SoftDiscretizedLogNumberFeatures() {}

protected:
  virtual double getValue(const Variable& input) const
  {
    double res = SoftDiscretizedNumberFeatures::getValue(input);
    return res > 0.0 ? log10(res) : -DBL_MAX;
  }

  virtual String getBoundaryName(size_t index) const
  {
    double boundary = pow(10.0, getBoundary(index));
    if (boundary < 1e-04 || boundary > 1e07)
      return String(boundary);
    if (boundary > 100)
      return String((int)boundary); // no decimals for simplicity
    int numberOfDecimals = boundary > 10 ? 1 : (boundary > 1 ? 2 : (boundary > 0.01 ? 3 : 6));
    return String(boundary, numberOfDecimals);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_SOFT_DISCRETIZED_NUMBER_H_
