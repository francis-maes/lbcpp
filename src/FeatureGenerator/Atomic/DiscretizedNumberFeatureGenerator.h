/*-----------------------------------------.---------------------------------.
| Filename: DiscretizedNumberFeatureGen...h| Base class for discretize       |
| Author  : Francis Maes                   | number feature generators       |
| Started : 04/10/2010 13:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_DISCRETIZED_NUMBER_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_DISCRETIZED_NUMBER_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{
  
class DiscretizedNumberFeatureGenerator : public FeatureGenerator
{
public:
  DiscretizedNumberFeatureGenerator(double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : minimumValue(minimumValue), maximumValue(maximumValue), numIntervals(numIntervals), doOutOfBoundsFeatures(doOutOfBoundsFeatures), isDouble(true)
  {
    jassert(maximumValue > minimumValue);
    jassert(numIntervals > 1);
  }

  DiscretizedNumberFeatureGenerator()
    : minimumValue(0.0), maximumValue(0.0), numIntervals(0), doOutOfBoundsFeatures(false) {}

  virtual EnumerationPtr createDiscreteNumberFeatures(ExecutionContext& context) = 0;

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    if (!checkNumInputs(context, 1))
      return EnumerationPtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    if (!inputVariable->getType()->inheritsFrom(doubleType) && !inputVariable->getType()->inheritsFrom(integerType))
    {
      context.errorCallback(inputVariable->getType()->getName() + T(" is not a number"));
      return EnumerationPtr();
    }
    isDouble = inputVariable->getType()->inheritsFrom(doubleType);
    outputName = inputVariable->getName() + T("Discretized");
    outputShortName = inputVariable->getName() + T("d");
    elementsType = probabilityType;
    return createDiscreteNumberFeatures(context);
  }

  virtual bool isSparse() const
    {return true;}


protected:
  friend class DiscretizedNumberFeatureGeneratorClass;

  double minimumValue;
  double maximumValue;
  size_t numIntervals;
  bool doOutOfBoundsFeatures;

  bool isDouble;

  virtual double getValue(const Variable& input) const
    {jassert(input.exists()); return isDouble ? input.getDouble() : (double)input.getInteger();}

  virtual String getBoundaryName(size_t index) const
    {return String(getBoundary(index));}

  double getBoundary(size_t index) const
  {
    jassert(index <= numIntervals);
    return minimumValue + (maximumValue - minimumValue) * (double)index / numIntervals;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_DISCRETIZED_NUMBER_H_
