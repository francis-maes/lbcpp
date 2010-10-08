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

  virtual Variable run(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  ReturnCode train(InferencePtr inference, ContainerPtr examples);
  ReturnCode evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator);
  ReturnCode crossValidate(InferencePtr inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds);

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

  void callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode);

  juce_UseDebuggingNewOperator

protected:
  friend class DecoratorInference;
  friend class SequentialInference;
  friend class ParallelInference;
  
  virtual void preInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;
  virtual void postInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;

  virtual Variable runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  
  Variable callRunInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();
extern InferenceContextPtr multiThreadedInferenceContext(ThreadPoolPtr threadPool);
inline InferenceContextPtr multiThreadedInferenceContext(size_t numCpus)
  {return multiThreadedInferenceContext(new ThreadPool(numCpus));}

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
