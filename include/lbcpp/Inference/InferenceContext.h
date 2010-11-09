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
# include "../Function/ThreadPool.h"

namespace lbcpp
{

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class InferenceContext : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual Variable run(const InferencePtr& inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  ReturnCode train(InferencePtr inference, ContainerPtr trainingExamples, ContainerPtr validationExamples);
  ReturnCode train(InferencePtr inference, const InferenceBatchLearnerInputPtr& learnerInput);
  ReturnCode evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator);
  ReturnCode crossValidate(InferencePtr inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds);
  Variable predict(InferencePtr inference, const Variable& input);

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

  const std::vector<InferenceCallbackPtr>& getCallbacks() const
    {return callbacks;}

  void setCallbacks(const std::vector<InferenceCallbackPtr>& callbacks)
    {this->callbacks = callbacks;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DecoratorInference;
  friend class SequentialInference;
  friend class ParallelInference;
  
  virtual void preInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;
  virtual void postInference(const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;

  virtual Variable runDecoratorInference(DecoratorInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runSequentialInference(SequentialInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runParallelInference(ParallelInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  
  Variable callRunInference(const InferencePtr& inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  
  void callPreInference(InferenceContextWeakPtr context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode);
  void callPostInference(InferenceContextWeakPtr context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode);

private:
  friend class InferenceContextClass;

  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();
extern InferenceContextPtr multiThreadedInferenceContext(ThreadPoolPtr threadPool);
inline InferenceContextPtr multiThreadedInferenceContext(size_t numCpus)
  {return multiThreadedInferenceContext(new ThreadPool(numCpus));}

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
