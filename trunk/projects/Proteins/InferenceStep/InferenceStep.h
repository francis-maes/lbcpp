/*-----------------------------------------.---------------------------------.
| Filename: InferenceStep.h                | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_H_

# include "CommonObjectFunctions.h"

namespace lbcpp
{

class ObjectPair : public Object
{
public:
  ObjectPair(ObjectPtr first, ObjectPtr second)
    : first(first), second(second) {}
  ObjectPair() {}

  ObjectPtr getFirst() const
    {return first;}

  ObjectPtr getSecond() const
    {return second;}

protected:
  ObjectPtr first;
  ObjectPtr second;
};

class ObjectPairContainer : public ObjectContainer
{
public:
  virtual String getFirstClassName() const
    {return T("Object");}

  virtual String getSecondClassName() const
    {return T("Object");}

  virtual std::pair<ObjectPtr, ObjectPtr> getPair(size_t index) const = 0;

  // ObjectContainer
  virtual String getContentClassName() const
    {return T("ObjectPair");}

  virtual ObjectPtr get(size_t index) const
  {
    std::pair<ObjectPtr, ObjectPtr> p = getPair(index);
    return new ObjectPair(p.first, p.second);
  }
};
typedef ReferenceCountedObjectPtr<ObjectPairContainer> ObjectPairContainerPtr;


class InferencePolicy;
typedef ReferenceCountedObjectPtr<InferencePolicy> InferencePolicyPtr;

class InferenceStep : public NameableObject
{
public:
  InferenceStep(const String& name) : NameableObject(name), loadedModificationTime(0) {}

  enum ResultCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  virtual ResultCode run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr& output) = 0;
/*
  virtual void updateModel(const File& model, ObjectContainerPtr examples, const Time& lastDataModificationTime)
  {
    if (loadedModificationTime != 0 && loadedModificationTime >= lastDataModificationTime)
      return; // the loaded model is up to date

    // try to load the model from file
    if (model.exists() && model.getLastModificationTime() >= lastDataModificationTime)
    {
      resetModel();
      loadModel(model);
      return;
    }
    
    // train model
    trainModel(examples);
    saveModel(model);
  }

  virtual void resetModel()
    {}

  virtual void trainModel(ObjectContainerPtr examples)
    {jassert(false);}
                        
  virtual void loadModel(const File& model)
    {jassert(false);}

  virtual void saveModel(const File& model)
    {jassert(false);}*/

private:
  Time loadedModificationTime;
};

class InferenceStep;
typedef ReferenceCountedObjectPtr<InferenceStep> InferenceStepPtr;

class ModelFreeInferenceStep : public InferenceStep
{
public:
  ModelFreeInferenceStep(const String& name) : InferenceStep(name) {}
  /*
  virtual void updateModel(const File& model, ObjectContainerPtr examples, const Time& lastDataModificationTime)
    {}

  virtual void loadModel(const File& model)
    {}

  virtual void saveModel(const File& model)
    {}*/
};

//////////////////////////////////////////////

class InferencePolicy : public NameableObject
{
public:
  typedef InferenceStep::ResultCode ResultCode;

  virtual ResultCode doSubStep(InferenceStepPtr step,  ObjectPtr input, ObjectPtr& output) = 0;
  virtual ResultCode doParallelSteps(const std::vector< std::pair<InferenceStepPtr, ObjectPtr> >& subInferences, ObjectContainerPtr output) = 0;

  virtual ResultCode doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureVectorPtr& result) = 0;
};

class DefaultInferencePolicy : public InferencePolicy
{
public:
  virtual ResultCode doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr& output)
    {return step->run(InferencePolicyPtr(this), input, output);}

  virtual ResultCode doParallelSteps(const std::vector< std::pair<InferenceStepPtr, ObjectPtr> >& subInferences, ObjectContainerPtr outputs)
  {
    jassert(outputs->size() == subInferences.size());
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      ObjectPtr output = outputs->get(i);
      ResultCode res = doSubStep(subInferences[i].first, subInferences[i].second, output);
      if (res != InferenceStep::finishedReturnCode)
        return res;
      outputs->set(i, output);
    }
    return InferenceStep::finishedReturnCode;
  }

  virtual ResultCode doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureVectorPtr& result)
  {
    FeatureDictionaryPtr dictionary = FeatureDictionaryManager::getInstance().getFlatVectorDictionary(classifier->getLabels());
    jassert(dictionary);
    SparseVectorPtr res = new SparseVector(dictionary);
    res->set(classifier->predict(input), 1.0);
    result = res;
    return InferenceStep::finishedReturnCode;
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
