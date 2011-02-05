/*------------------------------------------.---------------------------------.
 | Filename: SignedNumberFeatures.h         | A Feature Generator for         |
 | Author  : Francis Maes                   | signed numbers                  |
 | Started : 04/10/2010 12:33               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_SIGNED_NUMBER_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_SIGNED_NUMBER_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class SignedNumberFeatureGenerator : public FeatureGenerator
{
public:
  SignedNumberFeatureGenerator(FeatureGeneratorPtr positiveNumberFeatures = FeatureGeneratorPtr())
    : positiveNumberFeatures(positiveNumberFeatures), numBaseFeatures(0) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return positiveNumberFeatures->getRequiredInputType(0, 1);}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    if (!positiveNumberFeatures->initialize(context, inputVariables))
      return EnumerationPtr();

    numBaseFeatures = positiveNumberFeatures->getFeaturesEnumeration()->getNumElements();
    elementsType = probabilityType;

    DefaultEnumerationPtr res = new DefaultEnumeration(T("SignedNumberFeatures"));
    res->addElement(context, T("zero"));
    res->addElementsWithPrefix(context, positiveNumberFeatures->getFeaturesEnumeration(), T("positive"), T("+"));
    res->addElementsWithPrefix(context, positiveNumberFeatures->getFeaturesEnumeration(), T("negative"), T("-"));
    return res;
  }

  virtual bool isSparse() const
    {return true;}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const Variable& input = inputs[0];
    jassert(input.exists());
    double value = input.isDouble() ? input.getDouble() : (double)input.getInteger();
    const double epsilon = 1e-15;
    if (fabs(value) < epsilon)
      callback.sense(0, 1.0);
    else if (value > 0)
      callback.sense(1, positiveNumberFeatures, inputs, 1.0);
    else
    {
      Variable opposite(input.isInteger() ? Variable(-(int)value, input.getType()) : Variable(-value, input.getType()));
      callback.sense(1 + numBaseFeatures, positiveNumberFeatures, &opposite, 1.0);
    }
  }

protected:
  friend class SignedNumberFeatureGeneratorClass;

  FeatureGeneratorPtr positiveNumberFeatures;
  size_t numBaseFeatures;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_PERCEPTION_SIGNED_NUMBER_H_
