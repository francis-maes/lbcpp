/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Operator/FeatureGenerator.h>
using namespace lbcpp;

class SetInSparseDoubleObjectVariableGeneratorCallback : public VariableGeneratorCallback
{
public:
  SetInSparseDoubleObjectVariableGeneratorCallback(const SparseDoubleObjectPtr& target)
    : target(target) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(index, value);}

private:
  SparseDoubleObjectPtr target;
};

Variable FeatureGenerator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  SparseDoubleObjectPtr res(new SparseDoubleObject(getOutputType()));
  SetInSparseDoubleObjectVariableGeneratorCallback callback(res);
  computeVariables(inputs, callback);
  return res;
}
