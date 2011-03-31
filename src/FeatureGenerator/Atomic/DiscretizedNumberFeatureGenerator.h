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
    jassert(numIntervals);
  }

  DiscretizedNumberFeatureGenerator()
    : minimumValue(0.0), maximumValue(0.0), numIntervals(0), doOutOfBoundsFeatures(false) {}

  virtual EnumerationPtr createDiscreteNumberFeatures(ExecutionContext& context) = 0;

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return sumType(doubleType, integerType);}

  virtual String getOutputPostFix() const
    {return T("Discretized");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    const VariableSignaturePtr& inputVariable = inputVariables[0];
    isDouble = inputVariable->getType()->inheritsFrom(doubleType);
    if (!isDouble)
      jassertfalse;
    elementsType = probabilityType;
    return createDiscreteNumberFeatures(context);
  }

  //virtual bool isSparse() const
  //  {return true;}

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
