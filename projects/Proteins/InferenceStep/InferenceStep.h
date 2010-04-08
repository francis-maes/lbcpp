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

class InferencePolicy;
typedef ReferenceCountedObjectPtr<InferencePolicy> InferencePolicyPtr;
class InferenceVisitor;
typedef ReferenceCountedObjectPtr<InferenceVisitor> InferenceVisitorPtr;

class InferenceStep : public NameableObject
{
public:
  InferenceStep(const String& name = T("Unnamed"))
    : NameableObject(name), loadedModificationTime(0) {}

  virtual void accept(InferenceVisitorPtr visitor);

  enum ResultCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  virtual ResultCode run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr& output) = 0;

  ResultCode runOnSelfSupervisedExamples(InferencePolicyPtr policy, ObjectContainerPtr examples);
  ResultCode runOnSupervisedExamples(InferencePolicyPtr policy, ObjectContainerPtr examples);

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
//////////////////////////////////////////////
//////////////////////////////////////////////

class SequenceInferenceStep;
typedef ReferenceCountedObjectPtr<SequenceInferenceStep> SequenceInferenceStepPtr;

class InferenceVisitor
{
public:
  virtual ~InferenceVisitor() {}

  virtual void visit(InferenceStepPtr inference) = 0;
  virtual void visit(SequenceInferenceStepPtr inference) = 0;
};

//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

class InferencePolicy : public NameableObject
{
public:
  typedef InferenceStep::ResultCode ResultCode;

  virtual ResultCode doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr& output) = 0;

  virtual ResultCode doParallelSteps(const std::vector< std::pair<InferenceStepPtr, ObjectPtr> >& subInferences, ObjectContainerPtr output) = 0;

  virtual ResultCode doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr& result) = 0;
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

  virtual ResultCode doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr& result)
  {
    result = new Label(classifier->getLabels(), classifier->predict(input));
    return InferenceStep::finishedReturnCode;
  }
};


}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
