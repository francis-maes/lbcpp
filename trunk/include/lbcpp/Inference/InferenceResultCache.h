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
  InferenceStepResultCache(InferencePtr inference)
    : inference(inference) {}

  ObjectPtr get(ObjectPtr input) const;

  void add(ObjectPtr input, ObjectPtr output)
    {cache[input->getName()] = output;}

  InferencePtr getInference() const
    {return inference;}

private:
  InferencePtr inference;
  
  typedef std::map<String, ObjectPtr> InputOutputMap;
  InputOutputMap cache;
};

typedef ReferenceCountedObjectPtr<InferenceStepResultCache> InferenceStepResultCachePtr;

class InferenceResultCache : public Object
{
public:
  InferenceStepResultCachePtr getCacheForInferenceStep(InferencePtr inference) const;

  ObjectPtr get(InferencePtr inference, ObjectPtr input) const;

  void addStepCache(InferenceStepResultCachePtr stepCache);
  void add(InferencePtr inference, ObjectPtr input, ObjectPtr output);

private:
  typedef std::map<InferencePtr, InferenceStepResultCachePtr> CacheMap;
  CacheMap cache;
};

typedef ReferenceCountedObjectPtr<InferenceResultCache> InferenceResultCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_RESULT_CACHE_H_
