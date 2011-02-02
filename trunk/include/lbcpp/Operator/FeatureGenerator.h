/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Base class for Feature          |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_H_

# include "Operator.h"
# include "../Core/DynamicObject.h"

namespace lbcpp
{

class FeatureGenerator : public Operator
{
public:
  virtual double dotProduct(const Variable* inputs, size_t startIndex, double weight, const DenseDoubleObjectPtr& parameters) const
    {jassert(false); return 0.0;}

  virtual void computeFeatures(const Variable* inputs, size_t startIndex, double weight, const SparseDoubleObjectPtr& target) const = 0;

  virtual Variable computeOperator(const Variable* inputs) const
  {
    SparseDoubleObjectPtr res(new SparseDoubleObject(outputType));
    computeFeatures(inputs, 0, 1.0, res);
    return res;
  }
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

extern FeatureGeneratorPtr enumerationFeatureGenerator();
extern FeatureGeneratorPtr enumerationDistributionFeatureGenerator();

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_H_
