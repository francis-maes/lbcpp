/*-----------------------------------------.---------------------------------.
| Filename: HardDiscretizedNumberFeatu...h | Discretize a continuous number  |
| Author  : Julien Becker                  | into intervals                  |
| Started : 23/09/2010 14:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_HARD_DISCRETIZED_NUMBER_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_HARD_DISCRETIZED_NUMBER_H_

# include "DiscretizedNumberFeatureGenerator.h"

namespace lbcpp
{

class HardDiscretizedNumberFeatureGenerator : public DiscretizedNumberFeatureGenerator
{
public:
  HardDiscretizedNumberFeatureGenerator(double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : DiscretizedNumberFeatureGenerator(minimumValue, maximumValue, numIntervals, doOutOfBoundsFeatures) {}
  HardDiscretizedNumberFeatureGenerator() {}

  virtual EnumerationPtr createDiscreteNumberFeatures(ExecutionContext& context)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("HardDiscretizedNumberFeatures"));
    res->reserveElements((doOutOfBoundsFeatures ? 2 : 0) + numIntervals);
    if (doOutOfBoundsFeatures)
    {
      res->addElement(context, T("beyond ") + getBoundaryName(0));
      res->addElement(context, T("beyond ") + getBoundaryName(numIntervals));
    }
    for (size_t i = 0; i < numIntervals; ++i)
    {
      String name = T("in [") + getBoundaryName(i) + T(", ") + getBoundaryName(i + 1);
      name += i == numIntervals - 1 ? T("]") : T("[");
      res->addElement(context, name);
    }
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    double value = getValue(inputs[0]);
    if (value < minimumValue)
    {
      if (doOutOfBoundsFeatures)
        callback.sense(0, 1.0);
    }
    else if (value <= maximumValue)
    {
      size_t discretizedValue = (size_t)(numIntervals * (value - minimumValue) / (maximumValue - minimumValue));
      if (discretizedValue == numIntervals)
        --discretizedValue;
      callback.sense((doOutOfBoundsFeatures ? 2 : 0) + discretizedValue, 1.0);
    }
    else
    {
      if (doOutOfBoundsFeatures)
        callback.sense(1, 1.0);
    }
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_HARD_DISCRETIZED_NUMBER_H_
