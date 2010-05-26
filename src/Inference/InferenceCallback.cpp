/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.cpp          | Inference Callback              |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 12:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
#include "InferenceCallback/CancelAfterStepCallback.h"
#include "InferenceCallback/ExamplesCreatorCallback.h"
#include "InferenceCallback/CacheInferenceCallback.h"
#include "InferenceCallback/OnlineLearningInferenceCallback.h"
using namespace lbcpp;

InferenceCallbackPtr lbcpp::cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep)
  {return new CacheInferenceCallback(cache, parentStep);}

InferenceCallbackPtr lbcpp::cancelAfterStepCallback(InferencePtr lastStepBeforeBreak)
  {return new CancelAfterStepCallback(lastStepBeforeBreak);}

/*
** InferenceOnlineLearnerCallback
*/
void InferenceOnlineLearnerCallback::finishInferencesCallback()
  {learner->passFinishedCallback(inference);}

void InferenceOnlineLearnerCallback::postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  if (inference == stack->getCurrentInference() && supervision && output)
    learner->stepFinishedCallback(inference, input, supervision, output);
  else if (stack->getDepth() == 1)
    learner->episodeFinishedCallback(inference);
}

void declareInferenceCallbackClasses()
{
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}
