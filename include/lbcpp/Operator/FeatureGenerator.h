/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Base class for Feature          |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_H_

# include "VariableGenerator.h"
# include "../Data/DoubleVector.h"

namespace lbcpp
{

class FeatureGenerator : public VariableGenerator
{
public:
  virtual EnumerationPtr getFeaturesEnumeration(ExecutionContext& context, TypePtr& elementsType) = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  EnumerationPtr featuresEnumeration;
  TypePtr featuresType;

  TypePtr computeOutputType(ExecutionContext& context);
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

extern FunctionPtr concatenateDoubleVectorFunction(bool lazy);

extern FeatureGeneratorPtr enumerationFeatureGenerator();
extern FeatureGeneratorPtr enumerationDistributionFeatureGenerator();

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_H_
