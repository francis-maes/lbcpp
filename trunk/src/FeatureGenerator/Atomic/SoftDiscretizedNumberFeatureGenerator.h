/*-----------------------------------------.---------------------------------.
 | Filename: SoftDiscretizedNumberFeatu...h | A Feature Generator that        |
 | Author  : Francis Maes                   | discretized one feature.        |
 | Started : 24/09/2010 10:26               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERIC_ATOMIC_SOFT_DISCRETIZED_NUMBER_H_
# define LBCPP_FEATURE_GENERIC_ATOMIC_SOFT_DISCRETIZED_NUMBER_H_

# include "DiscretizedNumberFeatureGenerator.h"

namespace lbcpp
{

class SoftDiscretizedNumberFeatureGenerator : public DiscretizedNumberFeatureGenerator
{
public:
  SoftDiscretizedNumberFeatureGenerator(double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures, bool cyclicBehavior)
    : DiscretizedNumberFeatureGenerator(minimumValue, maximumValue, numIntervals, doOutOfBoundsFeatures), cyclicBehavior(cyclicBehavior)
  {
    jassert(!cyclicBehavior || !doOutOfBoundsFeatures); // out-of-bounds features are never active when using the cyclicBehavior
  }

  SoftDiscretizedNumberFeatureGenerator() : cyclicBehavior(false) {}
   
  virtual EnumerationPtr createDiscreteNumberFeatures(ExecutionContext& context)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("SoftDiscretizedNumberFeatures"));

    res->reserveElements((doOutOfBoundsFeatures ? 2 : 0) + numIntervals + (cyclicBehavior ? 0 : 1));
    if (doOutOfBoundsFeatures)
    {
      res->addElement(context, T("beyond ") + getBoundaryName(0));
      res->addElement(context, T("beyond ") + getBoundaryName(numIntervals));
    }
    for (size_t i = 0; i < numIntervals; ++i)
      res->addElement(context, T("close to ") + getBoundaryName(i));
    if (!cyclicBehavior)
      res->addElement(context, T("close to ") + getBoundaryName(numIntervals));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    if (!inputs[0].exists())
      return;
    double value = getValue(inputs[0]);
    double halfWidth = (maximumValue - minimumValue) / numIntervals;

    if (value <= minimumValue - halfWidth)
    {
      if (doOutOfBoundsFeatures)
        callback.sense(0, 1.0);
    }
    else if (value <= minimumValue)
    {
      if (doOutOfBoundsFeatures)
      {
        double k = (minimumValue - value) / halfWidth;
        callback.sense(0, k);
        callback.sense(2, 1 - k); // first interval feature
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
        callback.sense((doOutOfBoundsFeatures ? 2 : 0) + variable1, 1.0 - k);
      if (k > epsilon)
        callback.sense((doOutOfBoundsFeatures ? 2 : 0) + variable2, k);
    }
    else if (value <= maximumValue + halfWidth)
    {
      if (doOutOfBoundsFeatures)
      {
        double k = (value - maximumValue) / halfWidth;
        callback.sense(1, k);
        callback.sense(2 + numIntervals, 1 - k); // last interval feature
      }
    }
    else
    {
      if (doOutOfBoundsFeatures)
        callback.sense(1, 1.0);
    }
  }
  
private:
  friend class SoftDiscretizedNumberFeatureGeneratorClass;
  
  bool cyclicBehavior;
};
 
class SoftDiscretizedLogNumberFeatureGenerator : public SoftDiscretizedNumberFeatureGenerator
{
public:
  SoftDiscretizedLogNumberFeatureGenerator(double minimumLogValue, double maximumLogValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : SoftDiscretizedNumberFeatureGenerator(minimumLogValue, maximumLogValue, numIntervals, doOutOfBoundsFeatures, false) {}

  SoftDiscretizedLogNumberFeatureGenerator() {}

protected:
  virtual double getValue(const Variable& input) const
  {
    double res = SoftDiscretizedNumberFeatureGenerator::getValue(input);
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

#endif // !LBCPP_FEATURE_GENERIC_ATOMIC_SOFT_DISCRETIZED_NUMBER_H_
