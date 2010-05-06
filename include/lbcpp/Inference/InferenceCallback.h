/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.h            | Inference Callback base class   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_H_

# include "Inference.h"
# include "../ObjectPredeclarations.h"
# include "../Utilities/RandomVariable.h"

namespace lbcpp
{

class InferenceCallback : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual void startInferencesCallback(size_t count)
    {}

  virtual void finishInferencesCallback()
    {}

  // this function may modify the input or the supervision
  // it may also set an output, which causes the current inference step to be skipped
  // the function may also set a returnCode != Inference::finishedReturnCode to skip the inference step
  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}

  virtual void classificationCallback(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
    {}

  virtual void regressionCallback(InferenceStackPtr stack, RegressorPtr& regressor, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
    {}
};

class ScalarInferenceLearningCallback : public InferenceCallback
{
public:
  ScalarInferenceLearningCallback(LearnableAtomicInferencePtr step);

  virtual size_t postInferenceCallback(size_t epoch, FeatureGeneratorPtr features, double prediction, ScalarFunctionPtr loss) = 0;
  virtual size_t postEpisodeCallback() {return 0;}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);

protected:
  LearnableAtomicInferencePtr step;
  size_t epoch;
  ScalarVariableMean inputSize;
  InferencePtr currentParentStep;

  void updateInputSize(FeatureGeneratorPtr inputfeatures);
};

extern InferenceCallbackPtr cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep);
extern InferenceCallbackPtr cancelAfterStepCallback(InferencePtr lastStepBeforeBreak);
extern InferenceCallbackPtr stochasticScalarLinearInferenceLearningCallback(InferencePtr inference,
                                                                            IterationFunctionPtr learningRate,
                                                                            ScalarVectorFunctionPtr regularizer = ScalarVectorFunctionPtr(),
                                                                            bool normalizeLearningRate = true);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CALLBACK_H_
