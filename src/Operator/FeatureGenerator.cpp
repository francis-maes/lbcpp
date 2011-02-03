/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Operator/FeatureGenerator.h>
using namespace lbcpp;

class FillSparseVectorFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  FillSparseVectorFeatureGeneratorCallback(const SparseDoubleVectorPtr& target)
    : target(target) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
  {
    jassert(weight == 1.0);
    vector->appendTo(target, index);
  }

  virtual void sense(size_t index, const DoubleVectorPtr& vector)
    {sense(index, vector, 1.0);}

private:
  SparseDoubleVectorPtr target;
};

VariableSignaturePtr FeatureGenerator::initializeFunction(ExecutionContext& context)
{
  String outputName = T("Features");
  String outputShortName = T("phi");
  featuresType = doubleType;
  featuresEnumeration = initializeFeatures(context, featuresType, outputName, outputShortName);
  jassert(featuresEnumeration && featuresType);
  TypePtr outputType = lazyComputation ? getLazyOutputType(featuresEnumeration, featuresType) : getNonLazyOutputType(featuresEnumeration, featuresType);
  return new VariableSignature(outputType, outputName, outputShortName);
}

DoubleVectorPtr FeatureGenerator::toLazyVector(const Variable* inputs) const
  {return new LazyDoubleVector(refCountedPointerFromThis(this), inputs);}

DoubleVectorPtr FeatureGenerator::toComputedVector(const Variable* inputs) const
{
  SparseDoubleVectorPtr res(new SparseDoubleVector(getOutputType()));
  FillSparseVectorFeatureGeneratorCallback callback(res);
  computeFeatures(inputs, callback);
  return res;
}

Variable FeatureGenerator::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  if (lazyComputation)
    return toLazyVector(inputs);
  else
    return toComputedVector(inputs);
}
