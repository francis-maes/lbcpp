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

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    if (!checkNumInputs(context, 1) || !checkInputType(context, 0, booleanType))
      return EnumerationPtr();

    elementsType = probabilityType;
    outputName = inputVariables[0]->getName() + T("Features");
    outputShortName = inputVariables[0]->getShortName() + T("f");
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
