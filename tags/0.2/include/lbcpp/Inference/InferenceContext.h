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

  virtual ObjectPtr runInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

  virtual ReturnCode train(InferencePtr inference, ObjectContainerPtr examples);

  virtual SequentialInferenceStatePtr makeSequentialInferenceInitialState(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual void makeSequentialInferenceNextState(SequentialInferencePtr inference, SequentialInferenceStatePtr state, ObjectPtr subOutput, ReturnCode& returnCode);

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
  
  virtual ObjectPtr runDecoratorInference(DecoratorInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runSequentialInference(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runParallelInference(ParallelInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  
  ObjectPtr callRunInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  void callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
