/*-----------------------------------------.---------------------------------.
| Filename: BooleanFeatureGenerator.h      | Boolean Features                |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2011 12:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_BOOLEAN_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_BOOLEAN_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class BooleanFeatureGenerator : public FeatureGenerator
{
public:
  BooleanFeatureGenerator(bool includeMissingValue = true)
    : includeMissingValue(includeMissingValue) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return booleanType;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    elementsType = probabilityType;
    return includeMissingValue ? falseTrueOrMissingEnumeration : falseOrTrueEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const Variable& input = inputs[0];
    jassert(input.isBoolean());
    if (input.isMissingValue())
    {
      if (includeMissingValue)
        callback.sense(2, 1.0);
    }
    else
      callback.sense(input.getBoolean() ? 1 : 0, 1.0);
  }

protected:
  friend class BooleanFeatureGeneratorClass;

  bool includeMissingValue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_BOOLEAN_H_
