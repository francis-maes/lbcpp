/*-----------------------------------------.---------------------------------.
| Filename: CacheInferenceCallback.h       | Callbacks to build/use an       |
| Author  : Francis Maes                   | input/output cache              |
| Started : 16/04/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_CACHE_CALLBACK_H_
# define LBCPP_INFERENCE_CONTEXT_CACHE_CALLBACK_H_

# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/Inference/InferenceStack.h>
# include <lbcpp/Inference/InferenceResultCache.h>

namespace lbcpp
{

class CacheInferenceCallback : public InferenceCallback
{
public:
  CacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep)
    : cache(cache), parentStep(parentStep) {}

  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (!output && stack->getParentInference() == parentStep)
      output = cache->get(stack->getCurrentInference(), input);
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getParentInference() == parentStep && returnCode == Inference::finishedReturnCode)
    {
      cache->add(stack->getCurrentInference(), input, output);
      jassert(cache->get(stack->getCurrentInference(), input));
    }
  }

private:
  InferenceResultCachePtr cache;
  InferencePtr parentStep;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_CACHE_CALLBACK_H_
