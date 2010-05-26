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
#include "InferenceCallback/StochasticGradientDescentLearningCallback.h"
#include "InferenceCallback/BatchGradientDescentLearningCallback.h"
#include "InferenceCallback/RandomizerLearningInferenceCallback.h"
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

static bool isRandomizationRequired(LearningInferenceCallback::UpdateFrequency learningUpdateFrequency, LearningInferenceCallback::UpdateFrequency randomizationFrequency)
{
  jassert(learningUpdateFrequency != LearningInferenceCallback::never);

  if (learningUpdateFrequency == LearningInferenceCallback::perStep)
    return false;
  if (learningUpdateFrequency == LearningInferenceCallback::perEpisode)
  {
    jassert(randomizationFrequency == LearningInferenceCallback::never || randomizationFrequency == LearningInferenceCallback::perEpisode);
    return randomizationFrequency == LearningInferenceCallback::perEpisode;
  }
  if (learningUpdateFrequency == LearningInferenceCallback::perPass)
  {
    jassert(randomizationFrequency == LearningInferenceCallback::never || randomizationFrequency == LearningInferenceCallback::perPass);
    return randomizationFrequency == LearningInferenceCallback::perPass;
  }
  if (learningUpdateFrequency >= LearningInferenceCallback::perStepMiniBatch)
    return true;
}

LearningInferenceCallbackPtr lbcpp::stochasticDescentLearningCallback(InferencePtr inference, 
                                                            LearningInferenceCallback::UpdateFrequency learningUpdateFrequency,
                                                            IterationFunctionPtr learningRate,
                                                            bool normalizeLearningRate,
                                                            LearningInferenceCallback::UpdateFrequency randomizationFrequency,
                                                            LearningInferenceCallback::UpdateFrequency regularizerUpdateFrequency,
                                                            ScalarVectorFunctionPtr regularizer)
{
  jassert(learningUpdateFrequency != LearningInferenceCallback::never);
  LearningInferenceCallbackPtr res;
  if (learningUpdateFrequency == LearningInferenceCallback::perStep)
    res = new StochasticGradientDescentLearningCallback(inference, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else
    res = new BatchGradientDescentLearningCallback(inference, learningUpdateFrequency,
                                              learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  return isRandomizationRequired(learningUpdateFrequency, randomizationFrequency) 
    ? LearningInferenceCallbackPtr(new RandomizerLearningInferenceCallback(inference, randomizationFrequency, res))
    : res;
}

void declareInferenceCallbackClasses()
{
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}
