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

namespace lbcpp
{

class InferenceStepResultCache : public Object
{
public:
  InferenceStepResultCache(InferenceStepPtr step)
    : step(step) {}

  ObjectPtr get(ObjectPtr input) const
  {
    InputOutputMap::const_iterator it = cache.find(input->getName());
    return it == cache.end() ? ObjectPtr() : it->second;
  }

  void add(ObjectPtr input, ObjectPtr output)
    {cache[input->getName()] = output;}

  InferenceStepPtr getStep() const
    {return step;}

private:
  InferenceStepPtr step;
  
  typedef std::map<String, ObjectPtr> InputOutputMap;
  InputOutputMap cache;
};

typedef ReferenceCountedObjectPtr<InferenceStepResultCache> InferenceStepResultCachePtr;

class InferenceResultCache : public Object
{
public:
  InferenceStepResultCachePtr getCacheForInferenceStep(InferenceStepPtr step) const
    {CacheMap::const_iterator it = cache.find(step); return it == cache.end() ? InferenceStepResultCachePtr() : it->second;}

  ObjectPtr get(InferenceStepPtr step, ObjectPtr input) const
  {
    InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(step);
    ObjectPtr res = stepCache ? stepCache->get(input) : ObjectPtr();
    //if (res)
    //  std::cout << "Use: " << step->getName() << " input: " << input->getName() << std::endl;
    return res;
  }

  void addStepCache(InferenceStepResultCachePtr stepCache)
    {cache[stepCache->getStep()] = stepCache;}

  void add(InferenceStepPtr step, ObjectPtr input, ObjectPtr output)
  {
    //std::cout << "Add: " << step->getName() << " input: " << input->getName() << " output: " << output->toString() << std::endl;
    InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(step);
    if (!stepCache)
      stepCache = cache[step] = new InferenceStepResultCache(step);
    stepCache->add(input, output);
  }

private:
  typedef std::map<InferenceStepPtr, InferenceStepResultCachePtr> CacheMap;
  CacheMap cache;
};

typedef ReferenceCountedObjectPtr<InferenceResultCache> InferenceResultCachePtr;

class MakeCacheInferenceCallback : public InferenceCallback
{
public:
  MakeCacheInferenceCallback(InferenceStepResultCachePtr stepCache)
    : stepCache(stepCache) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getCurrentInference() == stepCache->getStep() && returnCode != InferenceStep::errorReturnCode)
      stepCache->add(input, output);
  }

private:
  InferenceStepResultCachePtr stepCache;
};

class AutoSubStepsCacheInferenceCallback : public InferenceCallback
{
public:
  AutoSubStepsCacheInferenceCallback(InferenceResultCachePtr cache, InferenceStepPtr parentStep)
    : cache(cache), parentStep(parentStep) {}

  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (!output && stack->getParentInference() == parentStep)
      output = cache->get(stack->getCurrentInference(), input);
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getParentInference() == parentStep && returnCode == InferenceStep::finishedReturnCode)
    {
      cache->add(stack->getCurrentInference(), input, output);
      jassert(cache->get(stack->getCurrentInference(), input));
    }
  }

private:
  InferenceResultCachePtr cache;
  InferenceStepPtr parentStep;
};

class UseCacheInferenceCallback : public InferenceCallback
{
public:
  UseCacheInferenceCallback(InferenceResultCachePtr cache)
    : cache(cache) {}
  
  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (!output)
      output = cache->get(stack->getCurrentInference(), input);
  }

private:
  InferenceResultCachePtr cache;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_CACHE_CALLBACK_H_
