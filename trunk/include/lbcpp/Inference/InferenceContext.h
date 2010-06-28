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
# include "../ObjectPredeclarations.h"

namespace lbcpp
{

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class InferenceContext : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual Variable runInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  // new
  ReturnCode train(InferencePtr inference, VariableContainerPtr examples);
  ReturnCode evaluate(InferencePtr inference, VariableContainerPtr examples, EvaluatorPtr evaluator);

  // old 
  ReturnCode train(InferencePtr inference, ObjectContainerPtr examples);

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

protected:  
  friend class DecoratorInference;
  friend class SequentialInference;
  friend class ParallelInference;
  
  virtual Variable runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  
  Variable callRunInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  void callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
