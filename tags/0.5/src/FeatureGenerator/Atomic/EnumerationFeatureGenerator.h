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

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return enumValueType;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    enumeration = inputVariables[0]->getType().dynamicCast<Enumeration>();
    jassert(enumeration);
    elementsType = probabilityType;
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
