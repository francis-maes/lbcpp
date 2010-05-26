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

  virtual void visit(InferenceBatchLearnerPtr inference) = 0;

  virtual void visit(SequentialInferencePtr inference) = 0;
  virtual void visit(ParallelInferencePtr inference) = 0;
  virtual void visit(StaticParallelInferencePtr inference) = 0;
  virtual void visit(SharedParallelInferencePtr inference) = 0;
  virtual void visit(ParameterizedInferencePtr inference) {}

  virtual void visit(ClassificationInferenceStepPtr inference) {}
  virtual void visit(RegressionInferenceStepPtr inference) {}
};

class DefaultInferenceVisitor : public InferenceVisitor
{
public:
  virtual void visit(InferenceBatchLearnerPtr inference);

  virtual void visit(SequentialInferencePtr inference);
  virtual void visit(ParallelInferencePtr inference) {}
  virtual void visit(StaticParallelInferencePtr inference);
  virtual void visit(SharedParallelInferencePtr inference);

protected:
  InferenceStack stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_VISITOR_H_
