/*-----------------------------------------.---------------------------------.
| Filename: LinearInference.h              | Linear Scalar Inference         |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 14:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LINEAR_H_
# define LBCPP_INFERENCE_LINEAR_H_

# include "NumericalInference.h"
# include <lbcpp/Function/PerceptionMaths.h>
# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class LinearInference : public NumericalInference
{
public:
  LinearInference(const String& name, PerceptionPtr perception)
    : NumericalInference(name, perception)/*, dotProductCache(NULL)*/ {}
  LinearInference() /*: dotProductCache(NULL)*/ {}

  virtual ~LinearInference()
    {clearDotProductCache();}

  virtual TypePtr getSupervisionType() const
    {return Class::get(T("ScalarFunction"));}

  virtual TypePtr getOutputType(TypePtr ) const
    {return doubleType();}

  virtual void beginRunSession()
    {clearDotProductCache(); /*if (parameters) dotProductCache = new FeatureGenerator::DotProductCache();*/}

  virtual void endRunSession()
    {clearDotProductCache();}
  
  virtual void validateParametersChange()
    {clearDotProductCache();}

  void clearDotProductCache()
  {
    /*if (dotProductCache)
    {
      delete dotProductCache;
      dotProductCache = NULL;
    }*/
  }
/*
  virtual FeatureGeneratorPtr getExampleGradient(const Variable& input, const Variable& supervision, const Variable& prediction, double& lossValue)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    ScalarFunctionPtr lossFunction = supervision.dynamicCast<ScalarFunction>();
    jassert(features && lossFunction);
    double lossDerivative;
    lossFunction->compute(prediction ? prediction.getDouble() : 0.0, &lossValue, &lossDerivative);
    return lbcpp::multiplyByScalar(features, lossDerivative);  
  }*/

  virtual void computeAndAddGradient(ObjectPtr& target, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue)
  {
    ScalarFunctionPtr lossFunction = supervision.dynamicCast<ScalarFunction>();
    jassert(lossFunction);
    double lossDerivative;
    lossFunction->compute(prediction ? prediction.getDouble() : 0.0, &exampleLossValue, &lossDerivative);
   // std::cout << "computeAndAddGradient: prevL2=" << (target ? l2norm(target) : -1.0)
   //   << " w = " << weight << " loss = " << exampleLossValue << " lossDerivative = " << lossDerivative << " inputL2 =  " << l2norm(perception, input);
    lbcpp::addWeighted(target, perception, input, lossDerivative * weight);
   // std::cout << " newL2 = " << l2norm(target) << std::endl;
  }

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    return lbcpp::dotProduct(parameters, perception, input);
      /*
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    jassert(features);
    return Variable(features->dotProduct(parameters, dotProductCache));*/
  }

private:
  //FeatureGenerator::DotProductCache* dotProductCache;
};

typedef ReferenceCountedObjectPtr<LinearInference> LinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LINEAR_H_
