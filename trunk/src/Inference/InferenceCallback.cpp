/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.cpp          | Inference Callback              |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 12:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
#include "InferenceCallback/CancelAfterStepCallback.h"
#include "InferenceCallback/ExamplesCreatorCallback.h"
#include "InferenceCallback/CacheInferenceCallback.h"
#include "InferenceCallback/StochasticScalarLinearInferenceLearningCallback.h"
using namespace lbcpp;

/*
** ScalarInferenceLearningCallback
*/
ScalarInferenceLearningCallback::ScalarInferenceLearningCallback(LearnableAtomicInferencePtr step)
    : step(step), epoch(1) {}

void ScalarInferenceLearningCallback::postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  if (step == stack->getCurrentInference() && supervision && output)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
    ScalarPtr prediction = output.dynamicCast<Scalar>();
    jassert(features && loss && prediction);
    epoch += learningEpoch(epoch, features, prediction->getValue(), loss);
  }
}

void ScalarInferenceLearningCallback::updateInputSize(FeatureGeneratorPtr inputfeatures)
{
  // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
  if (inputSize.getCount() < 10 ||                         // every time until having 10 samples
      (inputSize.getCount() < 100 && (epoch % 10 == 0)) || // every 10 epochs until having 100 samples
      (epoch % 100 == 0))                                  // every 100 epochs after that
    inputSize.push((double)(inputfeatures->l1norm()));
}

// -

InferenceCallbackPtr lbcpp::cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep)
  {return new CacheInferenceCallback(cache, parentStep);}

InferenceCallbackPtr lbcpp::cancelAfterStepCallback(InferencePtr lastStepBeforeBreak)
  {return new CancelAfterStepCallback(lastStepBeforeBreak);}

InferenceCallbackPtr lbcpp::stochasticScalarLinearInferenceLearningCallback(InferencePtr inference,
                                                                            IterationFunctionPtr learningRate,
                                                                            ScalarVectorFunctionPtr regularizer,
                                                                            bool normalizeLearningRate)
  {return new StochasticScalarLinearInferenceLearningCallback(inference.dynamicCast<LearnableAtomicInference>(),
                  learningRate, regularizer, normalizeLearningRate);}


void declareInferenceCallbackClasses()
{
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}
