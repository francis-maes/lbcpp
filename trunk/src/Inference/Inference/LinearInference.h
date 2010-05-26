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
    {clearDotProductCache(); dotProductCache = new FeatureGenerator::DotProductCache();}

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

  virtual FeatureGeneratorPtr getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput, double& lossValue)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    ScalarFunctionPtr lossFunction = supervision.dynamicCast<ScalarFunction>();
    ScalarPtr prediction = predictedOutput.dynamicCast<Scalar>();
    jassert(features && lossFunction && prediction);
    double lossDerivative;
    lossFunction->compute(prediction->getValue(), &lossValue, &lossDerivative);
    return multiplyByScalar(features, lossDerivative);  
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    jassert(features);
    DenseVectorPtr parameters = getOrCreateParameters(features->getDictionary());
    return new Scalar(features->dotProduct(parameters, dotProductCache));
  }

private:
  FeatureGenerator::DotProductCache* dotProductCache;
};

typedef ReferenceCountedObjectPtr<LinearInference> LinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LINEAR_H_
