/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.h             | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_H_
# define LBCPP_INFERENCE_CONTEXT_H_

# include "../InferenceStep/InferenceStep.h"
# include "InferenceStack.h"

namespace lbcpp
{

class InferenceCallback;
typedef ReferenceCountedObjectPtr<InferenceCallback> InferenceCallbackPtr;

class InferenceContext : public Object
{
public:
  typedef InferenceStep::ReturnCode ReturnCode;

  /*
  ** High level operations
  */
  virtual ReturnCode runWithSelfSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples) = 0;
  virtual ReturnCode runWithSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples) = 0;
  virtual ReturnCode runWithUnsupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples) = 0;

  /*
  ** Low level operations
  */
  virtual ObjectPtr runInference(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runParallelInferences(ParallelInferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  virtual ObjectPtr runClassification(ClassificationInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

protected:
  void callStartInferences(size_t count);
  void callFinishInferences();
  void callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);
  void callClassification(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
