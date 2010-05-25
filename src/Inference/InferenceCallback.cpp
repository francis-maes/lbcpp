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
#include "InferenceCallback/PerStepGradientDescentLearningCallback.h"
#include "InferenceCallback/RandomizedPerStepGradientDescentLearningCallback.h"
using namespace lbcpp;

InferenceCallbackPtr lbcpp::cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep)
  {return new CacheInferenceCallback(cache, parentStep);}

InferenceCallbackPtr lbcpp::cancelAfterStepCallback(InferencePtr lastStepBeforeBreak)
  {return new CancelAfterStepCallback(lastStepBeforeBreak);}


/*
** LearningInferenceCallback
*/
void LearningInferenceCallback::finishInferencesCallback()
  {passFinishedCallback();}

void LearningInferenceCallback::postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  if (inference == stack->getCurrentInference() && supervision && output)
  {
    currentParentStep = stack->getParentInference();
    stepFinishedCallback(input, supervision, output);
  }
  else if (stack->getCurrentInference() == currentParentStep)
    episodeFinishedCallback();
}

LearningInferenceCallbackPtr lbcpp::stochasticDescentLearningCallback(InferencePtr inference, 
                                                            LearningInferenceCallback::UpdateFrequency learningUpdateFrequency,
                                                            IterationFunctionPtr learningRate,
                                                            bool normalizeLearningRate,
                                                            LearningInferenceCallback::UpdateFrequency randomizationFrequency,
                                                            LearningInferenceCallback::UpdateFrequency regularizerUpdateFrequency,
                                                            ScalarVectorFunctionPtr regularizer)
{
  bool hasRandomization = (randomizationFrequency != LearningInferenceCallback::never && randomizationFrequency != LearningInferenceCallback::perStep);
  if (learningUpdateFrequency == LearningInferenceCallback::perStep)
  {
    if (hasRandomization)
      return new RandomizedPerStepGradientDescentLearningCallback(inference, learningRate, normalizeLearningRate, randomizationFrequency, regularizerUpdateFrequency, regularizer);
    else
      return new PerStepGradientDescentLearningCallback(inference, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  }
  jassert(false); // Missing implementation
  return InferenceCallbackPtr();
}

void declareInferenceCallbackClasses()
{
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}
