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

namespace lbcpp
{

class FeatureGenerator : public VariableGenerator
{
public:
  virtual Variable computeOperator(const Variable* inputs) const;
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

extern FeatureGeneratorPtr concatenateFeatureGenerator();

extern FeatureGeneratorPtr enumerationFeatureGenerator();
extern FeatureGeneratorPtr enumerationDistributionFeatureGenerator();

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_H_
