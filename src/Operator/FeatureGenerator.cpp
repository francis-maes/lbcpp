/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Operator/FeatureGenerator.h>
using namespace lbcpp;

class SetInSparseDoubleVectorVariableGeneratorCallback : public VariableGeneratorCallback
{
public:
  SetInSparseDoubleVectorVariableGeneratorCallback(const SparseDoubleVectorPtr& target)
    : target(target) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(index, value);}

private:
  SparseDoubleVectorPtr target;
};

Variable FeatureGenerator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  SparseDoubleVectorPtr res(new SparseDoubleVector(getOutputType()));
  SetInSparseDoubleVectorVariableGeneratorCallback callback(res);
  computeVariables(inputs, callback);
  return res;
}

TypePtr FeatureGenerator::computeOutputType(ExecutionContext& context)
{
  featuresType = doubleType;
  featuresEnumeration = getFeaturesEnumeration(context, featuresType);
  return doubleVectorClass(featuresEnumeration, featuresType);
}
