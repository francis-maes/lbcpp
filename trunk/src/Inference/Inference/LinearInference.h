/*-----------------------------------------.---------------------------------.
| Filename: LinearInference.h              | Linear Scalar Inference         |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 14:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LINEAR_H_
# define LBCPP_INFERENCE_LINEAR_H_

# include <lbcpp/Inference/ParameterizedInference.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class LinearInference : public ParameterizedInference
{
public:
  LinearInference(const String& name)
    : ParameterizedInference(name), dotProductCache(NULL) {}
  LinearInference() : dotProductCache(NULL) {}

  virtual ~LinearInference()
    {clearDotProductCache();}

  virtual void beginRunSession()
    {clearDotProductCache(); if (parameters) dotProductCache = new FeatureGenerator::DotProductCache();}

  virtual void endRunSession()
    {clearDotProductCache();}
  
  virtual void validateParametersChange()
    {clearDotProductCache();}

  void clearDotProductCache()
  {
    if (dotProductCache)
    {
      delete dotProductCache;
      dotProductCache = NULL;
    }
  }

  double getScalar(ObjectPtr object) const
  {
    if (!object)
      return 0.0;
    ScalarPtr scalar = object.dynamicCast<Scalar>();
    jassert(scalar);
    return scalar->getValue();
  }

  virtual FeatureGeneratorPtr getExampleGradient(const Variable& input, const Variable& supervision, const Variable& prediction, double& lossValue)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    ScalarFunctionPtr lossFunction = supervision.dynamicCast<ScalarFunction>();
    jassert(features && lossFunction);
    double lossDerivative;
    lossFunction->compute(getScalar(prediction), &lossValue, &lossDerivative);
    return multiplyByScalar(features, lossDerivative);  
  }

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (!parameters)
      return Variable();
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    jassert(features);
    return new Scalar(features->dotProduct(parameters, dotProductCache));
  }

private:
  FeatureGenerator::DotProductCache* dotProductCache;
};

typedef ReferenceCountedObjectPtr<LinearInference> LinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LINEAR_H_
