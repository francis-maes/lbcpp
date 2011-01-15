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
# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Inference/InferenceResultCache.h>

namespace lbcpp
{

class CacheInferenceCallback : public ExecutionCallback
{
public:
  CacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep)
    : cache(cache), parentStep(parentStep) {}
  CacheInferenceCallback() {}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {
    jassert(false); // broken
    //if (!output.exists() && context.findParentFunction() == parentStep)
    //  output = Variable(cache->get(context.getCurrentFunction(), input.getObject()));
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {
    jassert(false); // broken
    /*if (stack->findParentFunction() == parentStep)
    {
      cache->add(function, input.getObject(), output.getObject());
      jassert(cache->get(function, input.getObject()));
    }*/
  }

private:
  InferenceResultCachePtr cache;
  InferencePtr parentStep;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_CACHE_CALLBACK_H_
