/*-----------------------------------------.---------------------------------.
| Filename: IntegerFeatureGenerator.h      | Single Integer Feature          |
| Author  : Julien Becker                  |                                 |
| Started : 11/07/2011 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_INTEGER_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_INTEGER_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class IntegerFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return integerType;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    elementsType = doubleType; // inputVariables[0]->getType();
    return singletonEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const Variable& input = inputs[0];
    if (input.exists())
      callback.sense(0, (double)input.getInteger());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_INTEGER_H_
