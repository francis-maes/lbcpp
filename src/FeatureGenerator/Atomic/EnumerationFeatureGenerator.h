/*-----------------------------------------.---------------------------------.
| Filename: EnumerationFeatureGenerator.h  | Enumeration Features            |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 15:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class EnumerationFeatureGenerator : public FeatureGenerator
{
public:
  EnumerationFeatureGenerator(bool includeMissingValue = true)
    : includeMissingValue(includeMissingValue) {}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    if (!checkNumInputs(context, 1))
      return EnumerationPtr();

    enumeration = getInputType(0).dynamicCast<Enumeration>();
    if (!enumeration)
    {
      context.errorCallback(T("Not an enumeration"));
      return EnumerationPtr();
    }
    elementsType = probabilityType;
    outputName = inputVariables[0]->getName() + T("Features");
    outputShortName = inputVariables[0]->getShortName() + T("f");
    return includeMissingValue ? addMissingToEnumerationEnumeration(enumeration) : enumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    size_t index = getIndex(*inputs);
    if (includeMissingValue || index < enumeration->getNumElements())
      callback.sense(index, 1.0);
  }

protected:
  friend class EnumerationFeatureGeneratorClass;
  bool includeMissingValue;

  EnumerationPtr enumeration;

  size_t getIndex(const Variable& input) const
  {
    int index = input.getInteger();
    jassert(index >= 0 && index <= (int)enumeration->getNumElements());
    return (size_t)index;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_ENUMERATION_H_
