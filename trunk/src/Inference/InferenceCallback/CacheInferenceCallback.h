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

  virtual void preExecutionCallback(const FunctionPtr& function, const Variable& input)
  {
    jassert(false); // broken
    //if (!output.exists() && context.getParentFunction() == parentStep)
    //  output = Variable(cache->get(context.getCurrentFunction(), input.getObject()));
  }

  virtual void postExecutionCallback(const FunctionPtr& function, const Variable& input, const Variable& output)
  {
    ExecutionContext& context = getContext();
    if (context.getParentFunction() == parentStep)
    {
      cache->add(context.getCurrentFunction(), input.getObject(), output.getObject());
      jassert(cache->get(context.getCurrentFunction(), input.getObject()));
    }
  }

private:
  InferenceResultCachePtr cache;
  InferencePtr parentStep;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_CACHE_CALLBACK_H_
