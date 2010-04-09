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

class InferenceStep;
typedef ReferenceCountedObjectPtr<InferenceStep> InferenceStepPtr;
class SequenceInferenceStep;
typedef ReferenceCountedObjectPtr<SequenceInferenceStep> SequenceInferenceStepPtr;
class ParallelInferenceStep;
typedef ReferenceCountedObjectPtr<ParallelInferenceStep> ParallelInferenceStepPtr;

class InferenceStep : public NameableObject
{
public:
  InferenceStep(const String& name = T("Unnamed"))
    : NameableObject(name), loadedModificationTime(0) {}

  virtual void accept(InferenceVisitorPtr visitor);

  enum ReturnCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  virtual ObjectPtr run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

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


class InferenceVisitor
{
public:
  virtual ~InferenceVisitor() {}

  virtual void visit(InferenceStepPtr inference) = 0;
  virtual void visit(SequenceInferenceStepPtr inference) = 0;
  virtual void visit(ParallelInferenceStepPtr inference) = 0;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
