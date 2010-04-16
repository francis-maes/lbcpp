/*-----------------------------------------.---------------------------------.
| Filename: InferenceStep.h                | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class InferenceVisitor;
typedef ReferenceCountedObjectPtr<InferenceVisitor> InferenceVisitorPtr;

class InferenceContext;
typedef ReferenceCountedObjectPtr<InferenceContext> InferenceContextPtr;

class InferenceStep;
typedef ReferenceCountedObjectPtr<InferenceStep> InferenceStepPtr;

class SequentialInferenceStep;
typedef ReferenceCountedObjectPtr<SequentialInferenceStep> SequentialInferenceStepPtr;

class ParallelInferenceStep;
typedef ReferenceCountedObjectPtr<ParallelInferenceStep> ParallelInferenceStepPtr;

class ClassificationInferenceStep;
typedef ReferenceCountedObjectPtr<ClassificationInferenceStep> ClassificationInferenceStepPtr;

class InferenceStep : public NameableObject
{
public:
  InferenceStep(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  virtual void accept(InferenceVisitorPtr visitor);

  enum ReturnCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

protected:
  friend class InferenceContext;
};

class InferenceStep;
typedef ReferenceCountedObjectPtr<InferenceStep> InferenceStepPtr;

//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

class InferenceVisitor
{
public:
  virtual ~InferenceVisitor() {}

  virtual void visit(InferenceStepPtr inference) = 0;
  virtual void visit(SequentialInferenceStepPtr inference) = 0;
  virtual void visit(ParallelInferenceStepPtr inference) = 0;
};
/*
class DefaultInferenceVisitor : public InferenceVisitor
{
public:
  virtual void visit(SequentialInferenceStepPtr inference)
  {
    for (size_t i = 0; i < inference->getNumSubSteps(); ++i)
      inference->getSubStep(i)->accept(InferenceVisitorPtr(this));
  }
};*/

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
