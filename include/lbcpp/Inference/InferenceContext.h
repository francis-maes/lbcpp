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

class InferenceContext : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual ObjectPtr runInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runSequentialInference(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runParallelInference(ParallelInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  
  virtual ReturnCode train(InferenceBatchLearnerPtr learner, InferencePtr inference, ObjectContainerPtr examples);

  virtual SequentialInferenceStatePtr makeSequentialInferenceInitialState(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual void makeSequentialInferenceNextState(SequentialInferencePtr inference, SequentialInferenceStatePtr state, ObjectPtr subOutput, ReturnCode& returnCode);


  // old:
  virtual ReturnCode runWithSupervisedExamples(InferencePtr inference, ObjectContainerPtr examples) = 0;
  virtual ReturnCode runWithUnsupervisedExamples(InferencePtr inference, ObjectContainerPtr examples) = 0;

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

protected:
  void callStartInferences(size_t count);
  void callFinishInferences();
  void callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
