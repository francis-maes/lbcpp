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

  /*
  ** High level operations
  */
  virtual ReturnCode runWithSupervisedExamples(InferencePtr inference, ObjectContainerPtr examples) = 0;
  virtual ReturnCode runWithUnsupervisedExamples(InferencePtr inference, ObjectContainerPtr examples) = 0;

  /*
  ** Low level operations
  */
  virtual ObjectPtr runInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr runParallelInferences(ParallelInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  virtual ObjectPtr runClassification(ClassificationInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  virtual ObjectPtr runRegression(RegressionInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

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
  void callClassification(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode);
  void callRegression(InferenceStackPtr stack, RegressorPtr& regressor, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
