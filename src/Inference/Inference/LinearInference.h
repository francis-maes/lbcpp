/*-----------------------------------------.---------------------------------.
| Filename: LinearInference.h              | Linear Scalar Inference         |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 14:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LINEAR_H_
# define LBCPP_INFERENCE_LINEAR_H_

# include <lbcpp/Inference/InferenceBaseClasses.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

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

  virtual FeatureGeneratorPtr getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput, double& lossValue)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    ScalarFunctionPtr lossFunction = supervision.dynamicCast<ScalarFunction>();
    jassert(features && lossFunction);
    double lossDerivative;
    lossFunction->compute(getScalar(predictedOutput), &lossValue, &lossDerivative);
    return multiplyByScalar(features, lossDerivative);  
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (!parameters)
      return ObjectPtr();
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
