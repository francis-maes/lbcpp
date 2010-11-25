/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.h             | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_H_
# define LBCPP_INFERENCE_CONTEXT_H_

# include "Inference.h"
# include "../Execution/ThreadPool.h"
# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

extern Variable runInference(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision);

extern bool train(ExecutionContext& context, const InferencePtr& inference, ContainerPtr trainingExamples, ContainerPtr validationExamples);
extern bool train(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& learnerInput);
extern bool evaluate(ExecutionContext& context, const InferencePtr& inference, ContainerPtr examples, EvaluatorPtr evaluator);
extern bool crossValidate(ExecutionContext& context, const InferencePtr& inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds);
extern Variable predict(ExecutionContext& context, const InferencePtr& inference, const Variable& input);

class InferenceContext : public ExecutionContext
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual Variable runInference(const InferencePtr& inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  ReturnCode train(InferencePtr inference, ContainerPtr trainingExamples, ContainerPtr validationExamples);
  ReturnCode train(InferencePtr inference, const InferenceBatchLearnerInputPtr& learnerInput);
  ReturnCode evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator);
  ReturnCode crossValidate(InferencePtr inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds);
  Variable predict(InferencePtr inference, const Variable& input);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DecoratorInference;
  friend class SequentialInference;
  friend class ParallelInference;
  
  virtual void preInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;
  virtual void postInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;

  Variable callRunInference(const InferencePtr& inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  
  void callPreInference(InferenceContext& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode);
  void callPostInference(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode);
};

extern InferenceContextPtr singleThreadedInferenceContext();
inline InferenceContextPtr multiThreadedInferenceContext(ThreadPoolPtr threadPool)
  {return singleThreadedInferenceContext();} // FIXME
inline InferenceContextPtr multiThreadedInferenceContext(size_t numCpus)
  {return singleThreadedInferenceContext();} // FIXME

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
