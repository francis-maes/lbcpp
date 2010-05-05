/*-----------------------------------------.---------------------------------.
| Filename: InferenceResultCache.h         | A cache of (input,output) pairs |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 12:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_RESULT_CACHE_H_
# define LBCPP_INFERENCE_RESULT_CACHE_H_

# include "Inference.h"

namespace lbcpp
{

class InferenceStepResultCache : public Object
{
public:
  InferenceStepResultCache(InferenceStepPtr step)
    : step(step) {}

  ObjectPtr get(ObjectPtr input) const;

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
  InferenceStepResultCachePtr getCacheForInferenceStep(InferenceStepPtr step) const;

  ObjectPtr get(InferenceStepPtr step, ObjectPtr input) const;

  void addStepCache(InferenceStepResultCachePtr stepCache);
  void add(InferenceStepPtr step, ObjectPtr input, ObjectPtr output);

private:
  typedef std::map<InferenceStepPtr, InferenceStepResultCachePtr> CacheMap;
  CacheMap cache;
};

typedef ReferenceCountedObjectPtr<InferenceResultCache> InferenceResultCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_RESULT_CACHE_H_
