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
#include "InferenceCallback/MiniBatchGradientDescentLearningCallback.h"
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
    stepFinishedCallback(input, supervision, output);
  else if (stack->getDepth() == 1)
    episodeFinishedCallback();
}

static bool isRandomizationRequired(LearningInferenceCallback::UpdateFrequency learningUpdateFrequency, LearningInferenceCallback::UpdateFrequency randomizationFrequency)
{
  jassert(learningUpdateFrequency != LearningInferenceCallback::never);

  if (randomizationFrequency == LearningInferenceCallback::never ||
      randomizationFrequency == LearningInferenceCallback::perStep)
    return false;
  if (learningUpdateFrequency == LearningInferenceCallback::perStep ||
      learningUpdateFrequency >= LearningInferenceCallback::perStepMiniBatch)
    return true;
  if (learningUpdateFrequency == LearningInferenceCallback::perEpisode)
  {
    jassert(randomizationFrequency != LearningInferenceCallback::perPass); // this combination is not supported
    return false;
  }
  if (learningUpdateFrequency == LearningInferenceCallback::perPass)
    return false;
  jassert(false);
  return false;
}

LearningInferenceCallbackPtr lbcpp::stochasticDescentLearningCallback(LearnableAtomicInferencePtr inference, 
                                                            LearningInferenceCallback::UpdateFrequency randomizationFrequency,
                                                            LearningInferenceCallback::UpdateFrequency learningUpdateFrequency,
                                                            IterationFunctionPtr learningRate,
                                                            bool normalizeLearningRate,
                                                            LearningInferenceCallback::UpdateFrequency regularizerUpdateFrequency,
                                                            ScalarVectorFunctionPtr regularizer)
{
  jassert(learningUpdateFrequency != LearningInferenceCallback::never);
  LearningInferenceCallbackPtr res;

  size_t miniBatchSize = 0;
  if (learningUpdateFrequency >= LearningInferenceCallback::perStepMiniBatch)
  {
    miniBatchSize = learningUpdateFrequency - LearningInferenceCallback::perStepMiniBatch;
    if (miniBatchSize <= 1)
      learningUpdateFrequency = LearningInferenceCallback::perStep;
  }

  if (learningUpdateFrequency == LearningInferenceCallback::perStep)
    res = new StochasticGradientDescentLearningCallback(inference, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else if (learningUpdateFrequency >= LearningInferenceCallback::perStepMiniBatch && miniBatchSize < 100)
    res = new MiniBatchGradientDescentLearningCallback(inference, miniBatchSize, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
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
