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
using namespace lbcpp;

InferenceCallbackPtr lbcpp::cacheInferenceCallback(InferenceResultCachePtr cache, InferenceStepPtr parentStep)
  {return new CacheInferenceCallback(cache, parentStep);}

InferenceCallbackPtr lbcpp::cancelAfterStepCallback(InferenceStepPtr lastStepBeforeBreak)
  {return new CancelAfterStepCallback(lastStepBeforeBreak);}

void declareInferenceCallbackClasses()
{
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}
