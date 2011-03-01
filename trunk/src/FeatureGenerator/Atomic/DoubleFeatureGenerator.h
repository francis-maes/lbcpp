/*-----------------------------------------.---------------------------------.
| Filename: DoubleFeatureGenerator.h       | Single Double Feature           |
| Author  : Francis Maes                   |                                 |
| Started : 01/03/2011 18:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_DOUBLE_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_DOUBLE_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class DoubleFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    elementsType = inputVariables[0]->getType();
    return singletonEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const Variable& input = inputs[0];
    if (input.exists())
      callback.sense(0, input.getDouble());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_DOUBLE_H_
