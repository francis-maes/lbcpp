/*-----------------------------------------.---------------------------------.
| Filename: InferenceVisitor.h             | Inference visitor base class    |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 14:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_VISITOR_H_
# define LBCPP_INFERENCE_CONTEXT_VISITOR_H_

# include "InferenceStack.h"

namespace lbcpp
{

class InferenceVisitor : public ReferenceCountedObject
{
public:
  virtual ~InferenceVisitor() {}

  virtual void visit(SequentialInferenceStepPtr inference) = 0;
  virtual void visit(ParallelInferenceStepPtr inference) = 0;
  virtual void visit(SharedParallelInferenceStepPtr inference) = 0;
  virtual void visit(LearnableAtomicInferenceStepPtr inference) {}

  virtual void visit(ClassificationInferenceStepPtr inference) {}
  virtual void visit(RegressionInferenceStepPtr inference) {}
};

class DefaultInferenceVisitor : public InferenceVisitor
{
public:
  virtual void visit(SequentialInferenceStepPtr inference);
  virtual void visit(ParallelInferenceStepPtr inference) {}
  virtual void visit(SharedParallelInferenceStepPtr inference);

protected:
  InferenceStack stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_VISITOR_H_
